KEY_ALT		= 08h
KEY_CTRL	= 04h
KEY_SHIFT	= 03h

KEY_ESC		= 011Bh
KEY_BKSP    	= 0E08h
KEY_TAB        	= 0F09h
KEY_ENTER      	= 1C0Dh
KEY_SPACE      	= 3920h
KEY_HOME       	= 4700h
KEY_UP         	= 4800h
KEY_PGUP       	= 4900h
KEY_LEFT       	= 4B00h
KEY_RIGHT      	= 4D00h
KEY_END        	= 4F00h
KEY_DOWN       	= 5000h
KEY_PGDN       	= 5100h
KEY_INS        	= 5200h
KEY_DEL        	= 5300h

KEY_F1         	= 3B00h
KEY_F2         	= 3C00h
KEY_F3         	= 3D00h
KEY_F4         	= 3E00h
KEY_F5         	= 3F00h
KEY_F6		= 4000h
KEY_F7		= 4100h
KEY_F8		= 4200h
KEY_F9		= 4300h
KEY_F10		= 4400h
KEY_F11		= 8500h
KEY_F12		= 8600h

KEY_KPSTAR	= 372Ah
KEY_KPPLUS	= 4E2Bh
KEY_KPMIN	= 4A2Dh
KEY_KPENTER	= 0E00Dh

KEY_CTRLF1	= 5E00h
KEY_CTRLF2	= 5F00h
KEY_CTRLF3	= 6000h
KEY_CTRLF4	= 6100h
KEY_CTRLF5	= 6200h
KEY_CTRLF6	= 6300h
KEY_CTRLF7	= 6400h
KEY_CTRLF8	= 6500h
KEY_CTRLF9	= 6600h

KEY_CTRL0	= 0BFFh
KEY_CTRL1	= 02FFh
KEY_CTRL2	= 0300h
KEY_CTRL3	= 04FFh
KEY_CTRL4	= 05FFh
KEY_CTRL5	= 06FFh
KEY_CTRL6	= 071Eh
KEY_CTRL7	= 08FFh
KEY_CTRL8	= 091Bh
KEY_CTRL9	= 0A1Dh

KEY_CTRLA	= 1E01h
KEY_CTRLB       = 3002h
KEY_CTRLC       = 2E03h
KEY_CTRLD       = 2004h
KEY_CTRLE       = 1205h
KEY_CTRLF       = 2106h
KEY_CTRLG       = 2207h
KEY_CTRLH	= 2308h
KEY_CTRLI       = 1709h
KEY_CTRLJ       = 240Ah
KEY_CTRLK       = 250Bh
KEY_CTRLL       = 260Ch
KEY_CTRLM       = 320Dh
KEY_CTRLN       = 310Eh
KEY_CTRLO	= 180Fh
KEY_CTRLP	= 1910h
KEY_CTRLQ	= 1011h
KEY_CTRLR       = 1312h
KEY_CTRLS       = 1F13h
KEY_CTRLT       = 1414h
KEY_CTRLU       = 1615h
KEY_CTRLV       = 2F16h
KEY_CTRLW       = 1117h
KEY_CTRLX      	= 2D18h
KEY_CTRLY	= 1519h
KEY_CTRLZ       = 2C1Ah

KEY_CTRLINS	= 9200h
KEY_CTRLDEL	= 9300h
KEY_CTRLLEFT	= 7300h
KEY_CTRLRIGHT	= 7400h
KEY_CTRLUP	= 8D00h
KEY_CTRLDN	= 9100h
KEY_CTRLEND	= 7500h
KEY_CTRLHOME	= 7700h
KEY_CTRLPGUP	= 8400h
KEY_CTRLPGDN	= 7600h
KEY_CTRLTAB	= 9400h
KEY_CTRLENTER	= 1C0Ah
KEY_CTRLBKSP	= 0E7Fh

KEY_ALTF1	= 6800h
KEY_ALTF2	= 6900h
KEY_ALTF3	= 6A00h
KEY_ALTF4	= 6B00h
KEY_ALTF5	= 6C00h
KEY_ALTF6	= 6D00h
KEY_ALTF7	= 6E00h
KEY_ALTF8	= 6F00h
KEY_ALTF9	= 7000h

KEY_ALT0	= 8100h
KEY_ALT1	= 7800h
KEY_ALT2	= 7900h
KEY_ALT3	= 7A00h
KEY_ALT4	= 7B00h
KEY_ALT5	= 7C00h
KEY_ALT6	= 7D00h
KEY_ALT7	= 7E00h
KEY_ALT8	= 7F00h
KEY_ALT9	= 8000h

KEY_ALTUP	= 9800h
KEY_ALTDN	= 0A000h
KEY_ALTLEFT	= 9B00h
KEY_ALTRIGHT	= 9D00h
KEY_ALTC	= 2E00h
KEY_ALTL	= 2600h
KEY_ALTM	= 3200h
KEY_ALTP	= 1900h
KEY_ALTS	= 1F00h
KEY_ALTW	= 1100h
KEY_ALTX	= 2D00h

KEY_SHIFTF1	= 5400h
KEY_SHIFTF2	= 5500h
KEY_SHIFTF3	= 5600h
KEY_SHIFTF4	= 5700h
KEY_SHIFTF5	= 5800h
KEY_SHIFTF6	= 5900h
KEY_SHIFTF7	= 5A00h
KEY_SHIFTF8	= 5B00h
KEY_SHIFTF9	= 5C00h
KEY_SHIFTF10	= 5D00h

GLOBAL	keyshift:DWORD
GLOBALD	getkey
GLOBALD	getesc


