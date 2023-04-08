/*
 * usb.h
 *
 * Heavily based in usb_comms from libnx.
 *
 * Copyright (c) 2018-2020, Switchbrew and libnx contributors.
 * Copyright (c) 2020-2023, DarkMatterCore <pabloacurielz@gmail.com>.
 *
 * This file is part of nxdumptool (https://github.com/DarkMatterCore/nxdumptool).
 *
 * nxdumptool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nxdumptool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef __USB_H__
#define __USB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define USB_TRANSFER_BUFFER_SIZE    0x800000    /* 8 MiB. */

/// Used to indicate the USB speed selected by the host device.
typedef enum {
    UsbHostSpeed_None       = 0,
    UsbHostSpeed_FullSpeed  = 1,    ///< USB 1.x.
    UsbHostSpeed_HighSpeed  = 2,    ///< USB 2.0.
    UsbHostSpeed_SuperSpeed = 3     ///< USB 3.0.
} UsbHostSpeed;

/// Initializes the USB interface, input and output endpoints and allocates an internal transfer buffer.
bool usbInitialize(void);

/// Closes the USB interface, input and output endpoints and frees the transfer buffer.
void usbExit(void);

/// Returns a pointer to a dynamically allocated, page aligned memory buffer that's suitable for USB transfers.
void *usbAllocatePageAlignedBuffer(size_t size);

/// Used to check if the console has been connected to a USB host device and if a valid USB session has been established.
/// Returns a value from the UsbHostSpeed enum.
u8 usbIsReady(void);

/// Sends file properties to the host device before starting a file data transfer. Must be called before usbSendFileData().
/// If 'nsp_header_size' is greater than zero, NSP transfer mode will be enabled. The file will be treated as a NSP and this value will be taken as its full Partition FS header size.
/// Under NSP transfer mode, this function must be called right before transferring data from each NSP file entry to the host device, which should in turn write it all to the same output file.
/// Calling this function after NSP transfer mode has been enabled with a 'nsp_header_size' value greater than zero will result in an error.
/// The host device should immediately write 'nsp_header_size' padding at the start of the output file and start listening for further usbSendFileProperties() calls, or a usbSendNspHeader() call.
bool usbSendFileProperties(u64 file_size, const char *filename, u32 nsp_header_size);

/// Performs a file data transfer. Must be continuously called after usbSendFileProperties() until all file data has been transferred.
/// Data chunk size must not exceed USB_TRANSFER_BUFFER_SIZE.
/// If the last file data chunk is aligned to the endpoint max packet size, the host device should expect a Zero Length Termination (ZLT) packet.
bool usbSendFileData(void *data, u64 data_size);

/// Used to gracefully cancel an ongoing file transfer. The current USB session is kept alive.
void usbCancelFileTransfer(void);

/// Sends NSP header data to the host device, making it rewind the NSP file pointer to write this data, essentially finishing the NSP transfer process.
/// Must be called after the data from all NSP file entries has been transferred using both usbSendFileProperties() and usbSendFileData() calls.
/// If the NSP header size is aligned to the endpoint max packet size, the host device should expect a Zero Length Termination (ZLT) packet.
bool usbSendNspHeader(void *nsp_header, u32 nsp_header_size);

/// Nice and small wrapper for non-NSP files.
NX_INLINE bool usbSendFilePropertiesCommon(u64 file_size, const char *filename)
{
    return usbSendFileProperties(file_size, filename, 0);
}

#ifdef __cplusplus
}
#endif

#endif /* __USB_H__ */
