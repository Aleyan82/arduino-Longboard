/*
 * Copyright (c) 2016 Thomas Roell.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimers.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimers in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Thomas Roell, nor the names of its contributors
 *     may be used to endorse or promote products derived from this Software
 *     without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 */

#include "CDC.h"
#include "Arduino.h"
#include "wiring_private.h"

/* STM32L4x5/STM32L4x6 have USB_OTG_FS with a multi-packet FIFO. However 
 * to avoid sending ZLP packets, the CDC_TX_PACKET_SIZE is one byte
 * less than the maximum FIFO size in terms of 64 byte packets.
 */
#define CDC_TX_PACKET_SIZE (((USBD_CDC_FIFO_SIZE + 63) & ~63) -1)

stm32l4_usbd_cdc_t stm32l4_usbd_cdc;

CDC::CDC(struct _stm32l4_usbd_cdc_t *usbd_cdc, bool serialEvent)
{
    _usbd_cdc = usbd_cdc;

    _blocking = true;

    _rx_read = 0;
    _rx_write = 0;
    _rx_count = 0;
    _tx_read = 0;
    _tx_write = 0;
    _tx_size = 0;
    _tx_count = 0;
    _tx_total = 0;
  
    _transmitCallback = NULL;
    _receiveCallback = NULL;

    stm32l4_usbd_cdc_create(usbd_cdc);

    if (serialEvent) {
	serialEventCallback = serialEventDispatch;
    }
}

void CDC::begin(unsigned long baudrate)
{
    begin(baudrate, (uint8_t)SERIAL_8N1);
}

void CDC::begin(unsigned long baudrate, uint16_t config)
{
    /* If USBD_CDC has already been enabled/initialized by STDIO, just add the notify.
     */
    if (_usbd_cdc->state == USBD_CDC_STATE_INIT) {
	stm32l4_usbd_cdc_enable(_usbd_cdc, 0, CDC::_event_callback, (void*)this, (USBD_CDC_EVENT_RECEIVE | USBD_CDC_EVENT_TRANSMIT));
    } else {
	flush();

	stm32l4_usbd_cdc_notify(_usbd_cdc, CDC::_event_callback, (void*)this, (USBD_CDC_EVENT_RECEIVE | USBD_CDC_EVENT_TRANSMIT));
    }
}

void CDC::end()
{
    flush();

    stm32l4_usbd_cdc_disable(_usbd_cdc);
}

int CDC::available()
{
    return _rx_count;
}

int CDC::availableForWrite(void)
{
    if (_usbd_cdc->state < USBD_CDC_STATE_READY)
	return 0;

    return CDC_TX_BUFFER_SIZE - _tx_count;
}

int CDC::peek()
{
    if (_rx_count == 0) {
	return -1;
    }

    return _rx_data[_rx_read];
}

int CDC::read()
{
    unsigned int rx_read;
    uint8_t data;

    if (_rx_count == 0) {
	return -1;
    }

    rx_read = _rx_read;

    data = _rx_data[rx_read];
    
    _rx_read = (unsigned int)(rx_read + 1) & (CDC_RX_BUFFER_SIZE -1);

    armv7m_atomic_sub(&_rx_count, 1);
  
    return data;
}

size_t CDC::read(uint8_t *buffer, size_t size)
{
  unsigned int rx_read, rx_count;
  size_t count;

  count = 0;

  while (count < size) {

      rx_count = _rx_count;

      if (rx_count == 0) {
	  break;
      }

      rx_read = _rx_read;

      if (rx_count > (UART_RX_BUFFER_SIZE - rx_read)) {
	  rx_count = (UART_RX_BUFFER_SIZE - rx_read);
      }

      memcpy(&buffer[count], &_rx_data[rx_read], rx_count);
      count += rx_count;
      
      _rx_read = (rx_read + rx_count) & (CDC_RX_BUFFER_SIZE -1);

      armv7m_atomic_sub(&_rx_count, rx_count);
  }

  return count;
}

void CDC::flush()
{
    if (__get_IPSR() == 0) {
	while (_tx_count != 0) {
	    armv7m_core_yield();
	}
    
	while (!stm32l4_usbd_cdc_done(_usbd_cdc)) {
	    armv7m_core_yield();
	}
    }
}

size_t CDC::write(const uint8_t data)
{
    return write(&data, 1);
}

size_t CDC::write(const uint8_t *buffer, size_t size)
{
    unsigned int tx_read, tx_write, tx_count, tx_size;
    size_t count;

    if ((_usbd_cdc->state < USBD_CDC_STATE_READY) || !(stm32l4_usbd_cdc_info.lineState & 2)) {
	return 0;
    }

    // Clamp "size" if called from an ISR to avoid blocking
    if (!_blocking || (__get_IPSR() != 0)) {
	tx_count = _tx_count;

	if (size > (CDC_TX_BUFFER_SIZE - tx_count)) {
	    size = (CDC_TX_BUFFER_SIZE - tx_count);
	}
    }

    armv7m_atomic_add(&_tx_total, size);

    count = 0;

    while (count < size) {

	tx_count = CDC_TX_BUFFER_SIZE - _tx_count;

	if (tx_count == 0) {
	    // Here we cannot block from within an ISR because "size" got limited clamped

	    if (stm32l4_usbd_cdc_done(_usbd_cdc)) {
		tx_size = _tx_count;
		tx_read = _tx_read;

		if (tx_size > (CDC_TX_BUFFER_SIZE - tx_read)) {
		    tx_size = (CDC_TX_BUFFER_SIZE - tx_read);
		}
		
		if (tx_size > CDC_TX_PACKET_SIZE) {
		    tx_size = CDC_TX_PACKET_SIZE;
		}
		
		_tx_size = tx_size;
		
		stm32l4_usbd_cdc_transmit(_usbd_cdc, &_tx_data[tx_read], tx_size);
	    }

	    while (CDC_TX_BUFFER_SIZE == _tx_count) {
		armv7m_core_yield();
	    }

	    tx_count = CDC_TX_BUFFER_SIZE - _tx_count;
	}

	tx_write = _tx_write;

	if (tx_count > (CDC_TX_BUFFER_SIZE - tx_write)) {
	    tx_count = (CDC_TX_BUFFER_SIZE - tx_write);
	}

	if (tx_count > (size - count)) {
	    tx_count = (size - count);
	}

	memcpy(&_tx_data[tx_write], &buffer[count], tx_count);
	count += tx_count;
      
	_tx_write = (unsigned int)(tx_write + tx_count) & (CDC_TX_BUFFER_SIZE -1);

	armv7m_atomic_add(&_tx_count, tx_count);
    }

    if (__get_IPSR() == 0) {
	if (stm32l4_usbd_cdc_done(_usbd_cdc)) {
	    tx_size = _tx_count;
	    tx_read = _tx_read;

	    if (tx_size) {
		if (tx_size > (CDC_TX_BUFFER_SIZE - tx_read)) {
		    tx_size = (CDC_TX_BUFFER_SIZE - tx_read);
		}
	      
		if (tx_size > CDC_TX_PACKET_SIZE) {
		    tx_size = CDC_TX_PACKET_SIZE;
		}
	      
		_tx_size = tx_size;
	      
		stm32l4_usbd_cdc_transmit(_usbd_cdc, &_tx_data[tx_read], tx_size);
	    }
	}
    }

    return count;
}

void CDC::onTransmit(void(*callback)(void))
{
    _transmitCallback = callback;
}

void CDC::onReceive(void(*callback)(int))
{
    _receiveCallback = callback;
}

bool CDC::done()
{
    if (_tx_count) {
	return false;
    }

    if (!stm32l4_usbd_cdc_done(_usbd_cdc)) {
	return false;
    }

    return true;
}

void CDC::blockOnOverrun(bool block)
{
    _blocking = block;
}

bool CDC::isEnabled()
{
    return (_usbd_cdc->state >= USBD_CDC_STATE_READY);
}

void CDC::EventCallback(uint32_t events)
{
    unsigned int rx_write, rx_count, rx_size, count;
    unsigned int tx_read, tx_size;
    bool empty;

    if (events & USBD_CDC_EVENT_RECEIVE) {
	while (_rx_count != CDC_RX_BUFFER_SIZE) {
	    empty = (_rx_count == 0);

	    count = 0;

	    do {
		rx_count = CDC_RX_BUFFER_SIZE - _rx_count;

		if (rx_count == 0) {
		    break;
		}
	      
		rx_write = _rx_write;

		if (rx_count > (CDC_RX_BUFFER_SIZE - rx_write)) {
		    rx_count = (CDC_RX_BUFFER_SIZE - rx_write);
		}
	      
		rx_size = stm32l4_usbd_cdc_receive(_usbd_cdc, &_rx_data[rx_write], rx_count);
	      
		_rx_write = (rx_write + rx_size) & (CDC_RX_BUFFER_SIZE -1);
	      
		armv7m_atomic_add(&_rx_count, rx_size);
	      
		count += rx_size;
	      
	    } while (rx_size);
	  
	    if (empty && _receiveCallback) {
		(*_receiveCallback)(count);
	    }
	}
    }

    if (events & USBD_CDC_EVENT_TRANSMIT) {
	tx_size = _tx_size;
      
	_tx_read = (_tx_read + tx_size) & (CDC_TX_BUFFER_SIZE -1);
      
	armv7m_atomic_sub(&_tx_count, tx_size);
	armv7m_atomic_sub(&_tx_total, tx_size);
      
	_tx_size = 0;

	if (_tx_count != 0) {
	    tx_size = _tx_count;
	    tx_read = _tx_read;

	    if (tx_size > (CDC_TX_BUFFER_SIZE - tx_read)) {
		tx_size = (CDC_TX_BUFFER_SIZE - tx_read);
	    }
	  
	    if (tx_size > CDC_TX_PACKET_SIZE) {
		tx_size = CDC_TX_PACKET_SIZE;
	    }
	  
	    _tx_size = tx_size;
	  
	    stm32l4_usbd_cdc_transmit(_usbd_cdc, &_tx_data[tx_read], tx_size);
	}

	if ((_tx_total == 0) &&_transmitCallback) {
	    (*_transmitCallback)();
	}
    }
}

void CDC::_event_callback(void *context, uint32_t events)
{
    reinterpret_cast<class CDC*>(context)->EventCallback(events);
}

CDC::operator bool()
{
    return stm32l4_usbd_cdc_connected(_usbd_cdc);
}

unsigned long CDC::baud()
{
    return stm32l4_usbd_cdc_info.dwDTERate;
}

uint8_t CDC::stopbits()
{
    return stm32l4_usbd_cdc_info.bCharFormat;
}

uint8_t CDC::paritytype()
{
    return stm32l4_usbd_cdc_info.bParityType;
}

uint8_t CDC::numbits()
{
    return stm32l4_usbd_cdc_info.bDataBits;
}

bool CDC::dtr()
{
    return stm32l4_usbd_cdc_info.lineState & 1;
}

bool CDC::rts() 
{
    return stm32l4_usbd_cdc_info.lineState & 2;
}

extern void serialEvent() __attribute__((weak));

bool SerialUSB_empty() { return !SerialUSB.available(); }

CDC SerialUSB(&stm32l4_usbd_cdc, (serialEvent != NULL));
