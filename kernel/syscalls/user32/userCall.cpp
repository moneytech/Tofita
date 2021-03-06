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

namespace userCall {
bool userCallHandled(volatile process::Process *process, const TofitaSyscalls syscall) {
	volatile var frame = &process->frame;

	if (syscall == TofitaSyscalls::CreateWindowEx) {
		if (!probeForReadOkay(frame->rdxArg1, sizeof(CreateWindowExPayload)))
			return false;

		var payload = (CreateWindowExPayload *)frame->rdxArg1;

		if (probeForReadOkay((uint64_t)payload->lpClassName, 16))
			serialPrintf(L"<> lpClassName == %S\n", payload->lpClassName);

		if (probeForReadOkay((uint64_t)payload->lpWindowName, 16))
			serialPrintf(L"<> lpWindowName == %S\n", payload->lpWindowName);

		var window = dwm::OverlappedWindow_create(process->pid);

		window->hWnd = payload->hWnd;

		// TODO proper probe method, probably separate probeForWideCharOkay
		if (probeForReadOkay((uint64_t)payload->lpWindowName, 8)) {
			window->title = String::duplicate(payload->lpWindowName);
		}

		frame->raxReturn = window->windowId;
		process->schedulable = true;
		return true;
	}

	if (syscall == TofitaSyscalls::ShowWindow) {
		// TODO merge to probeForReadAndConvert os it takes <T> and returns nullable for `if let`
		if (!probeForReadOkay(frame->rdxArg1, sizeof(ShowWindowPayload)))
			return false;

		let payload = (ShowWindowPayload *)frame->rdxArg1;

		const uint64_t windowId = payload->windowId;
		const int32_t nCmdShow = payload->nCmdShow;
		var window = dwm::OverlappedWindow_find(process->pid, windowId);

		if (window != null) {
			window->visible = true;
			dwm::OverlappedWindow_attach(windowId);
		}

		process->schedulable = true;
		return true;
	}

	if (syscall == TofitaSyscalls::GetMessage) {
		if (!probeForReadOkay(frame->rdxArg1, sizeof(GetMessagePayload)))
			return false;

		var payload = (GetMessagePayload *)frame->rdxArg1;

		if (!probeForWriteOkay((uint64_t)payload->msg, sizeof(wapi::Msg)))
			return false;

		frame->raxReturn = process::getMessage(process, payload);

		// Should sorts of loop forever
		if (frame->raxReturn) {
			process->schedulable = true;
		} else {
			// process->syscallToHandle = TofitaSyscalls::GetMessage;
			process->awaitsGetMessage = true;
		}

		return true;
	}

	if (syscall == TofitaSyscalls::PostMessage) {
		if (!probeForReadOkay(frame->rdxArg1, sizeof(PostMessagePayload)))
			return false;

		var payload = (PostMessagePayload *)frame->rdxArg1;

		frame->raxReturn = process::postMessage(process, payload);

		process->schedulable = true;
		return true;
	}

	if (syscall == TofitaSyscalls::SwapWindowFramebuffer) {
		if (!probeForReadOkay(frame->rdxArg1, sizeof(SwapWindowFramebufferPayload)))
			return false;

		let payload = (SwapWindowFramebufferPayload *)frame->rdxArg1;

		if (!probeForWriteOkay((uint64_t)payload->fb, sizeof(nj::WindowFramebuffer)))
			return false;

		uint64_t windowId = payload->windowId;

		var fb = (nj::WindowFramebuffer *)payload->fb;
		var window = dwm::OverlappedWindow_find(process->pid, windowId);

		if (window == null)
			return false;

		let width = (uint32_t)window->width;
		let height = (uint32_t)window->height;
		let bytes = width * height * sizeof(Pixel32);

		if (window->fbZeta != null) {
			window->fbCurrentZeta = !window->fbCurrentZeta;
			if (window->fbCurrentZeta) {
				tmemcpy((void *)&window->fbZeta->pixels, (const void *)&window->fbGama->pixels, bytes);
				fb->pixels = (nj::Pixel32 *)&window->fbZeta->pixels;
				fb->width = window->fbZeta->width;
				fb->height = window->fbZeta->height;
			} else {
				tmemcpy((void *)&window->fbGama->pixels, (const void *)&window->fbZeta->pixels, bytes);
				fb->pixels = (nj::Pixel32 *)&window->fbGama->pixels;
				fb->width = window->fbGama->width;
				fb->height = window->fbGama->height;
			}
		}

		//		if (window != null) {
		//			var pixelsKernel = window->fbKernel->pixels;
		//			var pixelsUser = window->fbUser->pixels;
		//			let count = window->fbKernel->width * window->fbKernel->height;
		//			for (uint32_t i = 0; i < count; ++i)
		//			{
		//				pixelsKernel[i] = pixelsUser[i];
		//			}
		//		}

		// frame->raxReturn = 0;
		process->schedulable = true;
		return true;
	}

	if (syscall == TofitaSyscalls::GetOrCreateWindowFramebuffer) {
		if (!probeForReadOkay(frame->rdxArg1, sizeof(SwapWindowFramebufferPayload)))
			return false;

		let payload = (SwapWindowFramebufferPayload *)frame->rdxArg1;

		// TODO probeForWriteOkay alias taking pointer probeForWriteAtOkay
		if (!probeForWriteOkay((uint64_t)payload->fb, sizeof(nj::WindowFramebuffer)))
			return false;

		uint64_t windowId = payload->windowId;

		var fb = payload->fb;
		var window = dwm::OverlappedWindow_find(process->pid, windowId);

		if (window == null)
			return false;

		// TODO resize fb *here* on window size change
		if (window->fbZeta == null) {
			let width = (uint32_t)window->width;
			let height = (uint32_t)window->height;

			let bytes = width * height * sizeof(Pixel32);
			// serialPrintln(L"[GetOrCreateWindowFramebuffer.allocateBytes]");
			// window->fbZeta.pixels = (nj::Pixel32 *)PhysicalAllocator::allocateBytes(bytes);
			// window->fbGama.pixels = (nj::Pixel32 *)PhysicalAllocator::allocateBytes(bytes);

			// TODO client width
			window->fbZeta = allocateBitmap(width, height);
			window->fbGama = allocateBitmap(width, height);

			memset((void *)window->fbZeta->pixels, 0x33, bytes);
			memset((void *)window->fbGama->pixels, 0x33, bytes);

			// window->fbZeta.width = width; // TODO client width
			// window->fbZeta.height = height;
			// window->fbGama.width = width; // TODO client width
			// window->fbGama.height = height;
		}

		if (window->fbCurrentZeta) {
			fb->pixels = (nj::Pixel32 *)&window->fbZeta->pixels;
			fb->width = window->fbZeta->width;
			fb->height = window->fbZeta->height;
		} else {
			fb->pixels = (nj::Pixel32 *)&window->fbGama->pixels;
			fb->width = window->fbGama->width;
			fb->height = window->fbGama->height;
		}

		// frame->raxReturn = 0;
		process->schedulable = true;
		return true;
	}

	return false;
}
} // namespace userCall
