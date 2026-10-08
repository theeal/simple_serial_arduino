#include "SimpleSerial.h"

SimpleSerial::SimpleSerial(Stream* serial_port, uint32_t timeout_ms)
    : _stream(serial_port),
      _has_new_packet(false),
      _is_escaped(false),
      _receiving(false),
      _rx_idx(0),
      _timeout_ms(timeout_ms),
      _last_rx_time(0),
      _timed_out(false) {
    _last_valid_packet.valid = false;
    _last_valid_packet.id = 0;
    _last_valid_packet.payload_len = 0;
}

void SimpleSerial::begin() {
    _last_rx_time = millis();
}

void SimpleSerial::setTimeout(uint32_t timeout_ms) {
    _timeout_ms = timeout_ms;
}

bool SimpleSerial::hasTimedOut() const {
    return _timed_out;
}

void SimpleSerial::resetData() {
    _last_valid_packet.valid = false;
    _last_valid_packet.id = 0;
    _last_valid_packet.payload_len = 0;
    memset(_last_valid_packet.payload, 0, sizeof(_last_valid_packet.payload));
    _has_new_packet = false;
}

void SimpleSerial::loop() {
    while (_stream && _stream->available() > 0) {
        uint8_t b = _stream->read();
        process_byte(b);
    }

    if (_timeout_ms > 0 && (millis() - _last_rx_time > _timeout_ms)) {
        if (!_timed_out) {
            _timed_out = true;
            resetData();
        }
    }
}

void SimpleSerial::process_byte(uint8_t b) {
    if (_is_escaped) {
        if (_receiving && _rx_idx < sizeof(_rx_buffer)) {
            _rx_buffer[_rx_idx++] = b;
        }
        _is_escaped = false;
        return;
    }

    if (b == ESC_BYTE) {
        _is_escaped = true;
        return;
    }

    if (b == START_BYTE) {
        _receiving = true;
        _rx_idx = 0;
        return;
    }

    if (b == END_BYTE && _receiving) {
        _receiving = false;
        if (_rx_idx >= 2) {
            uint8_t len = _rx_buffer[0];
            uint8_t id  = _rx_buffer[1];

            if (len == _rx_idx && (len - 2) <= 64) {
                _rx_packet.id = id;
                _rx_packet.payload_len = len - 2;
                memcpy(_rx_packet.payload, &_rx_buffer[2], _rx_packet.payload_len);
                _rx_packet.valid = true;

                _last_valid_packet = _rx_packet;
                _has_new_packet = true;
                _last_rx_time = millis();
                _timed_out = false;
            }
        }
        return;
    }

    if (_receiving && _rx_idx < sizeof(_rx_buffer)) {
        _rx_buffer[_rx_idx++] = b;
    }
}

void SimpleSerial::write_escaped(uint8_t b) {
    if (b == START_BYTE || b == END_BYTE || b == ESC_BYTE) {
        _stream->write(ESC_BYTE);
    }
    _stream->write(b);
}

bool SimpleSerial::send(uint8_t id, const uint8_t* payload, uint8_t len) {
    if (!_stream || len > 64) return false;

    uint8_t total_len = len + 2;
    _stream->write(START_BYTE);
    write_escaped(total_len);
    write_escaped(id);

    for (uint8_t i = 0; i < len; i++) {
        write_escaped(payload[i]);
    }

    _stream->write(END_BYTE);
    return true;
}

bool SimpleSerial::send_int(uint8_t id, int32_t val) {
    uint8_t buf[4];
    byte_conversion::int_2_bytes(val, buf);
    return send(id, buf, 4);
}

bool SimpleSerial::send_float(uint8_t id, float val) {
    uint8_t buf[4];
    byte_conversion::float_2_bytes(val, buf);
    return send(id, buf, 4);
}

bool SimpleSerial::send_bool_array(uint8_t id, const bool* bool_arr, uint16_t count) {
    uint8_t bytes[4];
    uint8_t byte_count = byte_conversion::bool_array_to_bytes(bool_arr, count, bytes);
    return send(id, bytes, byte_count);
}

bool SimpleSerial::available() {
    return _has_new_packet;
}

SimpleSerial::Packet SimpleSerial::read() {
    _has_new_packet = false;
    return _last_valid_packet;
}
