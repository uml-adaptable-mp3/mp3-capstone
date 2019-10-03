/// \author Lasse

#include <vo_stdio.h>
#include <volink.h>
#include <kernel.h>

u_int16 DecodeUCS2(register u_int16* write,register u_int16 guess);
u_int16 DecodeUTF8(register u_int16* write,register u_int16 guess);

DLLENTRY(DecodeString)

//
// DecodeString() receives ID3v2 text as an 8-bit character sequence (terminated by a double NULL).
// It should produce a valid Unicode string and overwrite the input buffer with UCS2 character codes.
// The parameter 'coding' is passed from the ID3v2 tag itself which should define the coding used in
// the input. We won't trust it blindly, though.
//

void DecodeString(register u_int16* string,register char coding) {
	switch(coding) {
	default: // basically this should be latin-1 but let's try if it matches UTF8 or UTF16
		if(DecodeUCS2(string,1)) { // try this first because it is faster
			return; // it was UTC2 (UTF16) and we converted it
		} else	if(DecodeUTF8(string,1)) {
			DecodeUTF8(string,0); // seems to be valid UTF8
		} // otherwise assume its latin-1 which is already a valid output, do nothing
		break;
	case 1: // should be UTF16
		DecodeUCS2(string,0);
		break;
	case 2: // should be UTF16 with BOM
		DecodeUCS2(string,0);
		break;
	case 3: // should be UTF8
		DecodeUTF8(string,0);
		break;
	}
}

ioresult init(char *parameters) { return S_OK; }

void fini(void) {}

//
// UCS2 decoder relies on byte order marker if it needs to guess
//
static u_int16 DecodeUCS2(register u_int16* write,register u_int16 guess) {
	u_int16* read=write;
	u_int16 bs0=0;
	u_int16 bs1=8;
	u_int16 bom=read[0]|(read[1]<<8);
	if(bom==0xFEFF) {
		read+=2;
	} else if(bom==0xFFFE) {
		bs0=8;
		bs1=0;
		read+=2;
	} else if(guess) {
		return 0;
	}
	while(1) {
		u_int16 c=(read[0]<<bs0)|(read[1]<<bs1);
		read+=2;
		write[0]=c;
		if(!c) {
			return 1;
		}
		++write;
	}
}

static u_int16 DecodeUTF8(register u_int16* write,register u_int16 guess) {
	u_int16* read=write;
	u_int16 expectedFollowUps=0;
	u_int16 result;
	u_int16 bom=read[0]|(read[1]<<8);
	if(bom==0xFFFE || bom==0xFEFF) {
		read+=2;
	}
	while(read[0]) {
		u_int16 c=read[0];	
		u_int16 leadingOnes;
		++read;
		leadingOnes=0;
		// count the number of high bits set
		while(c&0x80) {
			c<<=1;
			++leadingOnes;
		}
		c&=0xFF; // mask control bits, leave only payload
		if(expectedFollowUps) {
			// we are expecting character code 10XXXXXX
			if(leadingOnes!=1) {
				// invalid code
				return 0;
			}
			result<<=6;
			result|=c>>1;
			--expectedFollowUps;
			if(expectedFollowUps) { // not ready yet
				continue; // continue while(read[0])
			} // otherwise writes a character to the result
		} else if(leadingOnes) {
			expectedFollowUps=leadingOnes-1;
			if(!expectedFollowUps) {
				return 0; // fail
			}
			result=c>>(leadingOnes);
			continue;
		} else {
			result=c;
		}
		if(!guess) {
			write[0]=result;
			++write;
		}
	}
	if(!guess) { // terminate
		write[0]=0;
	}
	return 1;
}
