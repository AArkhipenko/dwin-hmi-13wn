/* -----------------------------------------------------------------------------
 * UART driver implementation for DWIN T5L (DGUS compatible).
 * Supports UART2–UART5, interrupt-driven RX, polling TX, and DGUS
 * protocol frame handling (0x82 / 0x83 commands with optional CRC).
 * ----------------------------------------------------------------------------- */
#include "uart.h"

__bit Crc_check_flog = 0;
__bit Response_flog = 0;
__bit g_in_download_mode = 0;
#if UART2_ENABLE
u8 __xdata R_u2[UART2_RX_LENTH];
volatile u8 __xdata R_OD2 = 0;  // Receive data flag
volatile u16 __xdata R_CN2 = 0; // Receive data count
volatile u8 __xdata T_O2 = 0;   // Receive data timeout
volatile __bit Busy2 = 0;       // Transmit busy flag
#endif

void Uart_Init(void)
{
#if UART2_ENABLE
    {
        u16 i = 1024 - FOSC / 64 / BAUD_UART2;
        SREL2H = (u8)(i >> 8); // Baud rate = FCLK/64*(1024-SREL)
        SREL2L = (u8)i;        //
        MUX_SEL |= 0x40;       // UART2 is led out, UART3 is not led out, and WDT is turned off
        ADCON = 0x80;          // Select SREL0H:L as the baud rate generator
        SCON2 = 0x50;          // Accept enable and mode settings
        PCON &= 0x7F;          // SMOD=0
        IEN0 |= 0X10;          // ES0=1 Serial port 2 receiving + sending interrupt
        EA = 1;
        R_OD2 = 0;
        R_CN2 = 0;
    }
#endif
}

void uart2_ISR(void) __interrupt(4)
{
#if UART2_ENABLE
    if (RI2) // receive interrupt
    {
        R_u2[R_CN2] = SBUF2; // store data
        SCON2 &= 0xFE;       // clear RI2
        R_OD2 = 1;           // set flag
        if (R_CN2 < UART2_RX_LENTH - 1)
            R_CN2++;
        T_O2 = 5; // timeout reload
    }
    if (TI2 == 1)
    {
        SCON2 &= 0xFD;
        Busy2 = 0;
    }
#endif
}

void uart_send_byte(u8 Uart_number, u8 Dat)
{
    if (Uart_number == 2)
    {
#if UART2_ENABLE
        while (Busy2)
            ;
        Busy2 = 1;
        SBUF2 = Dat;
#endif
    }
}

void uart_send_str(u8 Uart_number, u8 *str)
{
#if USE_CRC
    u8 arr[256];
    u8 len = 0;
    while (*str)
    {
        arr[len++] = *str;
        uart_send_byte(Uart_number, *str++);
    }
    u16 crc = crc16table(arr, len);
    uart_send_byte(Uart_number, (u8)(crc & 0xFF));
    uart_send_byte(Uart_number, (u8)(crc >> 8));

#else
    while (*str)
        uart_send_byte(Uart_number, *str++);
#endif
}

void uart_send_arr(u8 Uart_number, u8 *arr, u8 len)
{
#if USE_CRC
    for (u8 i = 0; i < len; i++)
        uart_send_byte(Uart_number, arr[i]);
    u16 crc = crc16table(arr, len);
    uart_send_byte(Uart_number, (u8)(crc & 0xFF));
    uart_send_byte(Uart_number, (u8)(crc >> 8));
#else
    for (u8 i = 0; i < len; i++)
        uart_send_byte(Uart_number, arr[i]);
#endif
}

void uart_send_word(u8 Uart_number, u16 data)
{
    uart_send_byte(Uart_number, (u8)(data >> 8));
    uart_send_byte(Uart_number, (u8)(data & 0x00FF));
}

void DGUS_MonitorAndSendUpdates(void)
{
    if (monitor_ms < 100)
        return;
    monitor_ms = 0;
    u16 change_flag = DGUS_Read_VP(0x0F00); // Variable change indication
    u16 var_length = DGUS_Read_VP(0x0F01);  // Variable length
    u16 last_addr = 0;

    DGUS_Write_VP(0x5000, change_flag + 10);

    // Only proceed if change_flag high-byte == 0x5A
    if (((u8)(change_flag >> 8)) == 0x5A)
    {
        u8 i = 0;
        u16 temp_val = 0;
        static __xdata u8 packet[100] = {0};

        // Build packet header
        // Format (DGUS 0x83 response-like):
        // [5A A5 | LEN | 0x83 | AddrH | AddrL | Words | Data(2*Words) | (CRC Lo | CRC Hi if CRC ON)]
        packet[0] = 0x5A;
        packet[1] = 0xA5;
        packet[2] = (((u8)var_length) << 1) + 4; // Length of payload
        packet[3] = 0x83;
        packet[4] = (u8)change_flag;       // AddrH (0x0F00 high-byte==0x5A, low-byte carries AddrH)
        packet[5] = (u8)(var_length >> 8); // AddrL (0x0F01 high-byte)
        packet[6] = (u8)var_length;        // Words  (0x0F01 low-byte)

        last_addr = ((u16)packet[4] << 8) | packet[5];
        // Read variable data from DGUS and append to packet
        for (i = 0; i < (u8)var_length; i++)
        {
            temp_val = DGUS_Read_VP(last_addr + i);
            packet[7 + 2 * i] = (u8)(temp_val >> 8);
            packet[8 + 2 * i] = (u8)(temp_val);
        }

#if USE_CRC
        // If CRC is enabled, LEN must include the CRC(2) and CRC is computed over [payload], i.e. from packet[3] for (LEN-2) bytes
        packet[2] = (u8)(packet[2] + 2u);
        {
            u16 crc = crc16table(packet + 3, (u16)(packet[2] - 2u));
            packet[3 + (u16)packet[2] - 2u] = (u8)(crc & 0xFF); // CRC LOW
            packet[3 + (u16)packet[2] - 1u] = (u8)(crc >> 8);   // CRC HIGH
        }
#endif

        // Send packet over enabled UART channels (send exactly LEN+3 bytes; do not append CRC here)
#if UART2_ENABLE
        for (u16 i = 0; i < (u16)packet[2] + 3u; i++)
            uart_send_byte(2, packet[i]);
#endif

        // Clear DGUS flags
        DGUS_Write_VP(0x0F00, 0);
        DGUS_Write_VP(0x0F01, 0);
    }
}

void DGUS_HandleCmd82(u8 uart, u8 *frame)
{
    if (Crc_check_flog == 0) // --- Case 1: No CRC check ---
    {
        DGUS_WriteBytes((frame[4] << 8) + frame[5], frame + 6, frame[2] - 3);

        if (Response_flog)
        {
            const u8 ack[] = {DTHD1, DTHD2, 0x03, 0x82, 0x4F, 0x4B};
            for (u8 i = 0; i < 6; i++)
                uart_send_byte(uart, ack[i]);
        }
    }
    else // --- Case 2: With CRC check ---
    {
        u16 crc_calc = crc16table(frame + 3, frame[2] - 2);

        // CRC is [low][high]
        u16 crc_recv = (u16)frame[frame[2] + 2] << 8 | frame[frame[2] + 1];

        if (crc_calc == crc_recv)
        {
            DGUS_WriteBytes((frame[4] << 8) + frame[5], frame + 6, frame[2] - 5);

            if (Response_flog)
            {
                u8 ack_crc[] = {DTHD1, DTHD2, 0x05, 0x82, 0x4F, 0x4B, 0x00, 0x00};

                // Calculate and append CRC
                u16 ack_crc_val = crc16table(ack_crc + 3, ack_crc[2] - 2);
                ack_crc[ack_crc[2] + 1] = (u8)(ack_crc_val & 0xFF); // low
                ack_crc[ack_crc[2] + 2] = (u8)(ack_crc_val >> 8);   // high
                for (u8 i = 0; i < ack_crc[2] + 3; i++)
                    uart_send_byte(uart, ack_crc[i]);
            }
        }
    }
}

void DGUS_HandleCmd83(u8 uart, u8 *response, const u8 *request)
{
    u16 addr;
    u8 words;

    if (Crc_check_flog == 0)
    { // ---- CRC OFF ----
        // İlk 7 baytı kopyala: 83, AddrH, AddrL, Words dahil
        for (u8 i = 0; i < 7; i++)
            response[i] = request[i];

        addr = ((u16)response[4] << 8) | response[5];
        words = response[6];

        DGUS_ReadBytes(addr, &response[7], words);

        response[2] = (u8)(2u * words + 4u); // LEN = 2*Words + 4
        for (u16 i = 0; i < (u16)response[2] + 3u; i++)
            uart_send_byte(uart, response[i]);
    }
    else
    {                        // ---- CRC ON ----
        u8 len = request[2]; // LEN alanı (payload+CRC)
        // CRC over [request+3 .. request+3+(len-2)-1]
        u16 crc_calc = crc16table((u8 *)(request + 3), (u16)(len - 2));
        u8 crc_lo = request[3 + (u16)len - 2];
        u8 crc_hi = request[3 + (u16)len - 1];
        u16 crc_recv = ((u16)crc_hi << 8) | crc_lo;

        if (crc_calc == crc_recv)
        {
            for (u8 i = 0; i < 7; i++)
                response[i] = request[i];

            addr = ((u16)response[4] << 8) | response[5];
            words = response[6];

            DGUS_ReadBytes(addr, &response[7], words);

            response[2] = (u8)(2u * words + 4u + 2u); // +2 CRC

            // Response CRC: over [response+3 .. +3+(LEN-2)-1]
            u16 resp_crc = crc16table(response + 3, (u16)(response[2] - 2));
            response[3 + (u16)response[2] - 2] = (u8)(resp_crc & 0xFF); // CRC LOW
            response[3 + (u16)response[2] - 1] = (u8)(resp_crc >> 8);   // CRC HIGH
            for (u16 i = 0; i < (u16)response[2] + 3u; i++)
                uart_send_byte(uart, response[i]);
        }
        // if CRC mismatch: silently ignore (kept for protocol compatibility)
    }
}

void DGUS_ParseUartFrame(u8 *Arr, u16 *Len, u8 uart, __bit resp, __bit crc_on)
{
    u16 N = 0;
    u16 total = *Len;
    static __xdata u8 frame[256];  // tek frame’lik güvenli buffer
    static __xdata u8 resp83[128]; // 83 için geçici cevap alanı

    while (N + 3u <= total)
    {
        if (Arr[N] != DTHD1 || Arr[N + 1] != DTHD2)
        {
            N++;
            continue;
        }

        u8 len = Arr[N + 2];
        u16 fbytes = (u16)len + 3u;
        if (fbytes > sizeof(frame))
            break; // too large: safety cutoff
        if (N + fbytes > total)
            break; // incomplete frame: try later

        for (u16 i = 0; i < fbytes; i++)
            frame[i] = Arr[N + i];

        Response_flog = resp;
        Crc_check_flog = crc_on;

        u16 addr = (u16)frame[4] << 8 | frame[5];
        if (frame[3] == 0x82)
        {
            DGUS_HandleCmd82(uart, frame);
        }
        else if (frame[3] == 0x83)
        {
            DGUS_HandleCmd83(uart, resp83, frame);
        }
        N += fbytes;
    }
}

void DGUS_ProcessAllUarts(void)
{

#if UART2_ENABLE
    if ((R_OD2) && (!T_O2))
    {
        DGUS_ParseUartFrame(R_u2, (u16 *)(&R_CN2), 2, RESPONSE_UART2, USE_CRC);
        R_OD2 = 0;
        R_CN2 = 0;
    }
#endif
}
