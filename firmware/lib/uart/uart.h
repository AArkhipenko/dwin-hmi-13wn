/* -----------------------------------------------------------------------------
 * UART driver header for DWIN T5L/T5L51 (8051-core) controller.
 * Defines prototypes, flags, and ISR declarations for UART2–UART5.
 * ----------------------------------------------------------------------------- */
#ifndef __UART_H__
#define __UART_H__

#include "t5l1.h"
#include "config.h"
#include "crc16.h"
#include "sys.h"
#include "timer.h"

extern __bit Response_flog;    // for response
extern __bit Auto_data_upload; // For automatic data upload
extern __bit Crc_check_flog;   // Crc check mark
extern __bit g_in_download_mode;
// uart.h
extern volatile __xdata u8 T_O2;

/**
 * @brief Initialize UART2, UART3, UART4, and UART5 based on configuration.
 *
 * This function sets up the specified UART channels with the configured baud rates,
 * modes, and interrupt settings. It also configures the necessary pins for TX/RX
 * functionality. Each UART channel is conditionally compiled based on the
 * corresponding `UARTx_ENABLE` macro.
 */
void Uart_Init(void);

/**
 * @brief Аппаратное прерывание UART2
 */
void uart2_ISR(void) __interrupt(4);

/**
 * @brief Send a single byte over the specified UART.
 *
 * This function waits until the UART is not busy, then writes the byte to the
 * appropriate SBUF register to initiate transmission.
 *
 * @param Uart_number UART channel number (2, 3, 4, or 5)
 * @param Dat         Byte to send
 */
void uart_send_byte(u8 Uart_number, u8 Dat);

/**
 * @brief Send a null-terminated string over the specified UART.
 *
 * If CRC is enabled (USE_CRC), it calculates the CRC16 of the string and appends
 * it to the transmission. The function controls the TX pins for UART4 and UART5
 * to manage half-duplex communication.
 *
 * @param Uart_number UART channel number (2, 3, 4, or 5)
 * @param str         Pointer to the null-terminated string to send
 */
void uart_send_str(u8 Uart_number, u8 *str);

/**
 * @brief Send an array of bytes over the specified UART.
 *
 * @param Uart_number UART channel number (2, 3, 4, or 5)
 * @param arr Pointer to the byte array to send
 * @param len Number of bytes to send from the array
 */
void uart_send_arr(u8 Uart_number, u8 *arr, u8 len);

/**
 * @brief Send a 16-bit word over the specified UART.
 *
 * @param Uart_number
 * @param data
 */
void uart_send_word(u8 Uart_number, u16 data);

/**
 * @brief Monitor DGUS variable change flag and send updated data over UART.
 *
 * This function continuously checks DGUS system registers (0x0F00, 0x0F01) for variable
 * change events. When a change is detected, it reads the updated data from DGUS RAM,
 * formats it into a communication packet (5A A5 protocol frame), and sends it through
 * all enabled UART channels. After transmission, it clears the DGUS update flags.
 *
 * @note Эта функция не используется. Но ее отсутсвие ломает аппратное прерывание.
 */
void DGUS_MonitorAndSendUpdates(void);

/**
 * @brief Handle DGUS "0x82" command received from UART.
 *
 * This function processes a command frame with command code 0x82:
 *  - If CRC check is disabled, it directly writes the received payload to DGUS RAM.
 *  - If CRC check is enabled, it verifies the CRC before writing.
 *  - If Response_flog is set, it sends an ACK response back on the same UART channel.
 *
 * @param uart UART channel number (e.g., 2, 3, 4, 5)
 * @param frame Pointer to received command frame buffer
 */
void DGUS_HandleCmd82(u8 uart, u8 *frame);



/**
 * @brief Handle DGUS "0x83" command received from UART.
 *
 * This function processes a command frame with command code 0x83 (read request):
 *  - If CRC check is disabled, it directly reads the requested data from DGUS RAM
 *    and sends it back in a formatted packet.
 *  - If CRC check is enabled, it verifies the CRC, reads the requested data,
 *    appends CRC to the response, and sends it back.
 * 
 * @note Handle DGUS "0x83" (read) command.
 * Frame (no CRC):  [5A A5 | LEN | 83 | ADDR_H | ADDR_L | WORDS]
 * Response (no CRC): [5A A5 | (2*WORDS+4) | 83 | ADDR_H | ADDR_L | WORDS | DATA...]
 * CRC'li çerçevede, CRC iki byte olarak sondadır (LOW, HIGH) ve
 * CRC hesabı [CMD..payload] (LEN-2 byte) üzerinde yapılır.
 *
 * @param uart     UART channel number (e.g., 2, 3, 4, 5)
 * @param response Buffer to build the response frame
 * @param request  Pointer to received request frame
 */
void DGUS_HandleCmd83(u8 uart, u8 *response, const u8 *request);

/**
 * @brief Parse and process incoming DGUS UART frames.
 *
 * This function scans the received buffer for valid DGUS frames (starting with 0x5A 0xA5),
 * identifies the command code (0x82 or 0x83), and dispatches to the corresponding
 * command handler. CRC and response flags are applied dynamically.
 *
 * @param rx_buf    Pointer to receive buffer
 * @param data_len  Pointer to variable holding number of received bytes
 * @param uart      UART channel number (e.g., 2, 3, 4, 5)
 * @param response  Response flag (1 = send response, 0 = silent)
 * @param crc_check CRC check flag (1 = verify CRC, 0 = no CRC)
 */
void DGUS_ParseUartFrame(u8 *rx_buf, u16 *data_len, u8 uart, __bit response, __bit crc_check);

/**
 * @brief Dispatch UART receive buffers for DGUS frame parsing.
 *
 * This function checks if any UART channel (2..5) has completed receiving a frame,
 * then calls DGUS_ParseUartFrame() with the proper parameters. After processing,
 * it clears the receive flags and resets the counters.
 */
void DGUS_ProcessAllUarts(void);
#endif