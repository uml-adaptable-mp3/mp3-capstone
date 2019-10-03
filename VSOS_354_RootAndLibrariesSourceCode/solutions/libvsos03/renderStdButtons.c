#include <stdbuttons.h>

auto void RenderStdButtons(register StdButton *buttons) {
	while (buttons->result) {
		RenderStdButton(buttons);
		buttons++;
	}
}