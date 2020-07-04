// The Tofita Kernel
// Copyright (C) 2020  Oleg Petrenko
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Quake-style console with ~

uint8_t haveToQuake = 0;
function handleKeyDown(uint8_t key);
function (*keyDownHandler)(uint8_t) = null;
function quakeHandleButtonDown(uint8_t key);
function qsod(const char8_t *format, const uint64_t extra, const uint64_t more);