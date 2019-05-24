#pragma once
#include <avr/interrupt.h>
#include "interrupt.h"
#include "calc.h"
#include "uart.h"
#include "layer3.c"

/* Interrupt A - Transmitter */
ISR(TIMER0_COMPA_vect)
{
	if ((timerA++) > INTERRUPT_PERIOD)
	{
		timerA = 0;
        
        /* STEP1. GENERATING CRC */
        if(tFlag == FLAG_GENERATING_CRC)
        {
            bufferClear(tCrcBuffer, 32);
            rightShift(tPayloadBuffer, *tDlcBuffer, 2);
            tPayloadBuffer[0] = *tDestination;
            tPayloadBuffer[1] = *tSource;
            generateCrc(tCrcBuffer, tPayloadBuffer, *tDlcBuffer, _polynomial);
       
            tCounter = 0;
            tFlag = FLAG_SENDING_PREAMBLE;
        }

        /* STEP2. SENDING PREAMBLE */
        else if(tFlag == FLAG_SENDING_PREAMBLE)
        {
            if(readBit(_preamble, tCounter)) SEND_DATA_ONE();
            else SEND_DATA_ZERO();

            if((++tCounter) >= 8)
            {
                tCounter = 0;
                tFlag = FLAG_SENDING_CRC;
            }
        }

        /* STEP3. SENDING CRC */
        else if(tFlag == FLAG_SENDING_CRC)
        {
            if(readBit(tCrcBuffer, tCounter)) SEND_DATA_ONE();
            else SEND_DATA_ZERO();
            
            if((++tCounter) >= 32)
            {
                tCounter = 0;
                tFlag = FLAG_SENDING_DLC;
            }
        }

        /* STEP4. SENDING DLC */
        else if(tFlag == FLAG_SENDING_DLC)
        {
            if(readBit(tDlcBuffer, tCounter)) SEND_DATA_ONE();
            else SEND_DATA_ZERO();

            if((++tCounter) >= 8)
            {
                tCounter = 0;
                tFlag = FLAG_SENDING_PAYLOAD;
            }
        }

        /* STEP5. SENDING PAYLOAD */
        else if(tFlag == FLAG_SENDING_PAYLOAD)
        {
            if(readBit(tPayloadBuffer, tCounter)) SEND_DATA_ONE();
            else SEND_DATA_ZERO();
            
            if((++tCounter) >= *tDlcBuffer)
            {
                tCounter = 0;
                tFlag = 9999;
            }
        }
	}
}

/* Pin-Change-Interrupt - Receiver */
ISR(PCINT2_vect)
{
    /* STEP1. DETECTING PREAMBLE */
    if(rFlag == FLAG_DETECTING_PREAMBLE)
    {
        updateBit(rQueue, (rCounter%8), receiveData());

        if(checkPreamble(*rQueue, *_preamble))
        {
            printBit(rQueue, 8);
            uart_transmit('\r');
            uart_changeLine();
            uart_transmit(' ');
            printMsg(logMsg_preamble, 17);
            uart_changeLine();
            uart_changeLine();

            *rQueue = 0;
            rCounter = 0;
            rFlag = FLAG_RECEIVING_CRC;
        }
    }

    /* STEP2. RECEIVING CRC */
    else if(rFlag == FLAG_RECEIVING_CRC)
    {
        updateBit(rFrame->crc, rCounter, receiveData());

        if((++rCounter) >= 32)
        {
            printBit(rFrame->crc, SIZE_OF_CRC);
            uart_transmit('\r');
            uart_changeLine();
            uart_transmit(' ');
            printMsg(logMsg_crc, 12);
            uart_changeLine();
            uart_changeLine();

            rCounter = 0;
            rFlag = FLAG_RECEIVING_DLC;
        }
    }

    /* STEP3. RECEIVING DLC */
    else if(rFlag == FLAG_RECEIVING_DLC)
    {
        updateBit(rFrame->dlc, rCounter, receiveData());
        
        if((++rCounter) >= SIZE_OF_DLC)
        {
            printBit(rFrame->dlc, SIZE_OF_DLC);
            uart_transmit('\r');
            uart_changeLine();
            uart_transmit(' ');
            printMsg(logMsg_dlc, 12);
            uart_changeLine();
            uart_changeLine();

            rCounter = 0;
            rFlag = FLAG_RECEIVING_PAYLOAD;
        }
    }

    /* STEP4. RECEIVING PAYLOAD */
    else if(rFlag == FLAG_RECEIVING_PAYLOAD)
    {
        updateBit(rFrame->payload, rCounter, receiveData());

        if((++rCounter) >= (*(rFrame->dlc)))
        {
            printBit(rFrame->payload, *(rFrame->dlc));
            uart_transmit('\r');
            uart_changeLine();
            uart_transmit(' ');
            printMsg(logMsg_payload, 16);
            uart_changeLine();
            uart_changeLine();
            
            rCounter = 0;
            rFlag = FLAG_CHECKING_CRC;
        }
    }

    /* STEP5. CHECKING CRC */
    else if(rFlag == FLAG_CHECKING_CRC)
    {
        if((checkCrc(rFrame->crc, rFrame->payload, *(rFrame->dlc), _polynomial)))
        {
            printBit(rFrame->crc, SIZE_OF_CRC);
            uart_changeLine();
            uart_transmit(' ');
            printMsg(logMsg_crc_true, 14);
            uart_changeLine();
            uart_changeLine();
        }
        else
        {
            printBit(rFrame->crc, SIZE_OF_CRC);
            uart_transmit('\r');
            uart_changeLine();
            uart_transmit(' ');
            printMsg(logMsg_crc_false, 16);
            uart_changeLine();
            uart_changeLine();
        }
        
        rCounter = 0;
        rFlag = FLAG_PROCESSING_DATA;
    }
    
    /* STEP6. LAYER3 */
    else if(rFlag == FLAG_PROCESSING_DATA)
    {
        layer3(rFrame);
    }
}

/* Clock Signal Interrupt */
ISR(TIMER0_COMPB_vect)
{
	if ((timerB++) > INTERRUPT_PERIOD)
	{
		timerB = 0;
        
        ////////////////////////////////////////////////////////
		/* CLOCK SIGNAL TRANSMIT */
		PIN_CHANGE();
        ////////////////////////////////////////////////////////
	}
}