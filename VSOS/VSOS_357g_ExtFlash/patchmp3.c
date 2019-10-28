#include <codecmpg.h>

enum CodecError CodMpgDecodePatch(struct Codec *codec,
				  struct CodecServices *cs,
				  const char **errorString);


struct Codec *KernelCodMpgCreate(void) {
	struct Codec *cod = CodMpgCreate();
	if (cod) {
		cod -> Decode = CodMpgDecodePatch;
	}
	return cod;
}

