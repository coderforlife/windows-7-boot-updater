/*
 * Windows 7 Boot Updater (github.com/coderforlife/windows-7-boot-updater)
 * Copyright (C) 2021  Jeffrey Bush - Coder for Life
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Animation.h"

#include "Bootres.h"
#include "PngConverter.h"
#include "UI.h"

using namespace Win7BootUpdater;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::IO;

#define ImageFlagsHasRealDPI (0x1000)

string Animation::DirDesc::get() {
	return UI::GetMessage(Msg::ItMustBeAFullActivityBmpOr105Frames);
}

inline static bool IsBitmap(Image ^i) { return i->GetType()->IsAssignableFrom(Bitmap::typeid); }

inline static Graphics ^CreateGraphics(int w, int h, Bitmap ^%b) {
	Graphics ^g = Graphics::FromImage(b = gcnew Bitmap(w, h, PixelFormat::Format32bppArgb));
	g->Clear(Animation::Transparent);
	return g;
}

inline static Graphics ^CreateGraphics(int w, int h, Color bg, Bitmap ^%b) {
	Graphics ^g = Graphics::FromImage(b = gcnew Bitmap(w, h, PixelFormat::Format24bppRgb));
	g->Clear(bg);
	return g;
}

Image ^Animation::CreateFromSingle(string file) {
	Bitmap ^src;
	try {
		src = gcnew Bitmap(file);
	} catch (Exception ^) {
		UI::ShowError(Msg::CouldNotOpenTheFileAsAnImageFile, file, Msg::OpeningAnimationError);
		return nullptr;
	}
	Image ^i = CreateFromSingle(src);
	delete src;
	return i;
}

Image ^Animation::CreateFromSingle(Bitmap ^src) {
	Bitmap ^b;
	Graphics ^g = CreateGraphics(Width, FullHeight, b);
	Rectangle srcRect(0, 0, src->Width, src->Height);
	CreateFromSingle(src, srcRect, b, g);
	return b;
}

void Animation::CreateFromSingle(Image ^src, Rectangle srcRect, Bitmap ^b, Graphics ^g) {
	if (IsBitmap(src) && src->Flags & ImageFlagsHasRealDPI) ((Bitmap^)src)->SetResolution(g->DpiX, g->DpiY);
	if (b->Flags   & ImageFlagsHasRealDPI) b->SetResolution(g->DpiX, g->DpiY);
	Rectangle destRect(0, 0, Width, Height);
	if (src->Width  < Width ) { destRect.X = (Width  - src->Width ) / 2; destRect.Width  = src->Width;  }
	if (src->Height < Height) { destRect.Y = (Height - src->Height) / 2; destRect.Height = src->Height;	}
	for (int i = 0; i < Frames; ++i) {
		g->DrawImage(src, destRect, srcRect, GraphicsUnit::Pixel);
		destRect.Y += Height;
	}
}

Image ^Animation::CreateFromFolder(string path) {
	// Look for a full activity.bmp file
	Bitmap ^full = nullptr;
	string activity_bmp = Path::Combine(path, Bootres::activity);
	if (File::Exists(activity_bmp)) {
		try {
			full = gcnew Bitmap(activity_bmp);
			if (full->Width != Width || full->Height != FullHeight) {
				delete full;
				full = nullptr;
			}
		} catch (Exception ^) {  }
	}

	// Create the bitmap that will hold the entire animation	
	Bitmap ^b;
	Graphics ^g = CreateGraphics(Width, FullHeight, b);

	// Attempt to compile an animation
	if (full == nullptr) {
		DirectoryInfo ^dir = nullptr;
		try {
			dir = gcnew DirectoryInfo(path);
		} catch (Exception ^) {
			UI::ShowError(Msg::TheDirectoryCouldNotBeOpened, path, Msg::OpeningAnimationError);
			return nullptr;
		}

		// Find all files
		List<String^> ^files = gcnew List<String^>();
		for (int i = 0; i < ext->Length; ++i) {
			for each (FileInfo ^fi in dir->GetFiles(L"*."+ext[i])) {
				files->Add(fi->FullName);
			}
		}
		files->Sort();

		// Special Cases
		if (files->Count == 0) {
			UI::ShowError(Msg::TheDirectoryDoesNotContainAnyAcceptableImageFiles, path, Msg::OpeningAnimationError);
			return nullptr;
		} else if (files->Count == 1) {
			return CreateFromSingle(files[0]);
		}

		// Draw each image to the animation
		Drawing::Rectangle srcRect(0, 0, Width, Height);
		Drawing::Rectangle destRect(0, 0, Width, Height);
		int total = Math::Min(Frames, files->Count);
		for (int i = 0; i < total; ++i) {
			Bitmap ^src = nullptr;
			try {
				src = gcnew Bitmap(files[i]);
				if (src->Flags & ImageFlagsHasRealDPI) src->SetResolution(g->DpiX, g->DpiY);
				srcRect.Width  = src->Width;
				srcRect.Height = src->Height;
				if (src->Width  < Width ) { destRect.X = (Width  - src->Width ) / 2; destRect.Width  = src->Width;  } else { destRect.X = 0; destRect.Width  = Width;  }
				if (src->Height < Height) { destRect.Y = (Height - src->Height) / 2; destRect.Height = src->Height;	} else { destRect.Y = 0; destRect.Height = Height; }
				destRect.Y += Height*i;
				g->DrawImage(src, destRect, srcRect, GraphicsUnit::Pixel);
			} catch (Exception ^) {
			} finally { if (src) delete src; }

		}
	} else {
		if (full->Flags & ImageFlagsHasRealDPI) full->SetResolution(g->DpiX, g->DpiY);
		g->DrawImageUnscaled(full, 0, 0);
		delete full;
	}

	return b;
}

Image ^Animation::ResolveTransparency(Image ^img, int width, int height, Color bg, Image ^animBgImg) {
	Bitmap ^b;
	Graphics ^g = CreateGraphics(width, height, bg, b);
	if (animBgImg && (animBgImg->Width > X && animBgImg->Height > Y)) {
		Rectangle srcRect(X, Y, Math::Min(Width, animBgImg->Width - X), Math::Min(Height, animBgImg->Height - Y));
		CreateFromSingle(animBgImg, srcRect, b, g);
	}
	g->DrawImageUnscaled(img, 0, 0);
	return b;
}

Image ^Animation::CreateFromData(array<Byte> ^data) {
	MemoryStream ^s = nullptr;
	Bitmap ^i = nullptr;
	Bitmap ^b = nullptr;
	Graphics ^g = nullptr;
	try {
		i = gcnew Bitmap(s = gcnew MemoryStream(data));
		g = CreateGraphics(i->Width, i->Height, b);
		if (i->Flags & ImageFlagsHasRealDPI) i->SetResolution(g->DpiX, g->DpiY);
		g->DrawImageUnscaled(i, 0, 0);
	} catch (Exception ^) {
		return nullptr;
	} finally {
		if (s) delete s;
		if (i) delete i;
		if (g) delete g;
	}
	return b;
}

array<Byte> ^Animation::GetPngData(Image ^img) {
	if (IsBitmap(img))
		return PngConverter::GetBytes((Bitmap^)img);
	MemoryStream ^s = gcnew MemoryStream();
	img->Save(s, Imaging::ImageFormat::Png);
	return s->ToArray();
}
