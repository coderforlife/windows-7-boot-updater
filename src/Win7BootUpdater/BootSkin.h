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

#pragma once

#include "MultipartFile.h"

namespace Win7BootUpdater {
	/// <remarks>Contains all the settings needed for a single file to be boot skinned (either Winload or Winresume).</remarks>
	PUBLIC ref class BootSkinFile sealed {
	private:
		bool winresume;
		
		bool defaultAnim, winloadAnim;
		System::Drawing::Image ^activity;

		int msgCount;
		array<string> ^msgs;
		array<System::Drawing::Font^> ^fonts;
		array<System::Drawing::SolidBrush^> ^textColors;
		array<int> ^positions;
		System::Drawing::SolidBrush ^msgBgColor, ^bgColor;

		System::Drawing::Image ^bg;

	internal:
		BootSkinFile(bool winresume);
		void Reset();
		void Load(System::Xml::XmlElement ^n, MultipartFile ^f);
		void Save(System::Xml::XmlTextWriter ^n, MultipartFile ^f);
		
		property array<int> ^TextSizes { array<int> ^get(); }
		property array<System::Drawing::Color> ^TextColors { array<System::Drawing::Color> ^get(); }
		property array<int> ^Positions { array<int> ^get(); }

	public:
		/// <summary>The default font used by the bootup process. Since this currently cannot be changed, this is used as a reference if you wish to draw what the boot skin will look like.</summary>
		literal string DefaultFont = L"Segoe UI";

		/// <summary>Checks if these settings are for winresume or winload</summary>
		/// <returns>true if these are settings for winresume, false if they are for winload</returns>
		bool IsWinresume();
		
		/// <returns>True if the default animation is used, false otherwise</returns>
		bool IsDefaultAnim();
		/// <returns>True if these are settings for winresume and it is going to use whatever animation winload will use</returns>
		bool IsWinloadAnim();
		/// <returns>True if the animation is not set so (same as <c><see cref="IsDefaultAnim()" /> || <see cref="IsWinloadAnim()" /></c>).</returns>
		bool AnimIsNotSet();
		/// <summary>Sets this file to use the default animation.</summary>
		void UseDefaultAnim();
		/// <summary>If these are settings for winresume, sets it to use whatever animation winload uses.</summary>
		void UseWinloadAnim();
		/// <summary>Sets the animation to be used. Check <see cref="AnimIsNotSet" /> first to see that there is an animation. Setting the animation will cause the default and winload animation flags to be cleared.</summary>
		property System::Drawing::Image ^Anim { System::Drawing::Image ^get(); void set(System::Drawing::Image ^value); }
		
		/// <returns>True if a full screen background image is used. This currently implies that no messages are shown.</returns>
		bool UsesBackgroundImage();
		/// <summary>
		/// Sets or gets the current full screen background image. Check <see cref="UsesBackgroundImage" /> first to see if there is an image. Setting this to <c>null</c> removes the full screen background image.
		/// Since having a full screen background image currently implies that no messages are shown (but this may be changed in the future) you should always set <see cref="MessageCount" /> to 0 when setting this to something besides <c>null</c>.
		/// </summary>
		property System::Drawing::Image ^Background { System::Drawing::Image ^get(); void set(System::Drawing::Image ^value); }

		/// <summary>The background color of the entire bootup process. This is adjusted with <see cref="WinXXX::GetClosestBGColor(System::Drawing::Color)" />.</summary>
		property System::Drawing::Color BackColor { void set(System::Drawing::Color x); System::Drawing::Color get(); }
		/// <summary>The background color behind the messages.</summary>
		property System::Drawing::Color MessageBackColor { System::Drawing::Color get(); void set(System::Drawing::Color value); }

		/// <summary>The number of messages to show, either 0, 1, or 2.</summary>
		property int MessageCount { int get(); void set(int value); }
		
		/// <summary>Gets the text of a message</summary>
		/// <param name="i">Either 0 or 1</param>
		/// <returns>The message text</returns>
		property string Message[unsigned long] { string get(unsigned long i); void set(unsigned long i, string x); }
		/// <summary>Gets the text size of a message</summary>
		/// <param name="i">Either 0 or 1</param>
		/// <returns>The message text size</returns>
		property int TextSize[unsigned long] { int get(unsigned long i); void set(unsigned long i, int x); }
		/// <summary>Gets the text color of a message</summary>
		/// <param name="i">Either 0 or 1</param>
		/// <returns>The message text color</returns>
		property System::Drawing::Color TextColor[unsigned long] { System::Drawing::Color get(unsigned long i); void set(unsigned long i, System::Drawing::Color x); }
		/// <summary>Gets the position of a message</summary>
		/// <param name="i">Either 0 or 1</param>
		/// <returns>The message position along the y-axis</returns>
		property int Position[unsigned long] { int get(unsigned long i); void set(unsigned long i, int x); }

		/// <summary>A brush that paints with <see cref="BackColor" />. This is meant for efficiency while drawing so that new brushes don't have to be allocated.</summary>
		property System::Drawing::Brush ^BackBrush { System::Drawing::Brush ^get(); }
		/// <summary>A brush that paints with <see cref="MessageBackColor" />. This is meant for efficiency while drawing so that new brushes don't have to be allocated.</summary>
		property System::Drawing::Brush ^MessageBackBrush { System::Drawing::Brush ^get(); }
		/// <summary>A brush that paints with <see cref="TextColor" />. This is meant for efficiency while drawing so that new brushes don't have to be allocated.</summary>
		/// <param name="i">Either 0 or 1</param>
		/// <returns>The message text brush</returns>
		property System::Drawing::Brush ^TextBrush[unsigned long] { System::Drawing::Brush ^get(unsigned long i); }
		/// <summary>The font to use for writing <see cref="Message" />.</summary>
		/// <param name="i">Either 0 or 1</param>
		/// <returns>The message font, this incorporates <see cref="TextSize" /> and <see cref="DefaultFont" /></returns>

		property System::Drawing::Font ^Font[unsigned long] { System::Drawing::Font ^get(unsigned long i); }
	};

	/// <remarks>Contains all the settings needed for a boot skin. Can easily be load from and saved to a file.</remarks>
	PUBLIC ref class BootSkin sealed {
	private:
		static System::Xml::XmlReaderSettings ^settings;
		BootSkinFile ^winload, ^winresume;

		void Save(System::IO::Stream ^data, MultipartFile ^f);

		string Load(System::IO::Stream ^data, bool uncompressed);
		string Load(System::IO::Stream ^data, MultipartFile ^f);

	public:
		/// <summary>Creates a new boot skin with default settings</summary>
		BootSkin();
		/// <summary>Forces a boot skin to take on default settings</summary>
		void Reset();


		/// <summary>Loads a boot skin from a set of winload and winresume files</summary>
		/// <param name="winload">The winload path to load from</param>
		/// <param name="winresume">The winresume path to load from, if this is null, then the winload file is used for all properties</param>
		void Load(string winload, string winresume);


		/// <summary>Loads a boot skin from a file</summary>
		/// <param name="file">The file path to load from</param>
		/// <returns>A string containing an error message if the file could not be completely loaded, or null if it succeeded to load</returns>
		string Load(string file);
		/// <summary>Loads a boot skin from a stream</summary>
		/// <param name="data">The stream to load from</param>
		/// <returns>A string containing an error message if the file could not be completely loaded, or null if it succeeded to load</returns>
		string Load(System::IO::Stream ^data);

		
		/// <summary>Saves a boot skin to a file</summary>
		/// <param name="file">The file path to save to</param>
		/// <returns>A string containing an error message if the file could not be written, or null if it succeeded to save</returns>
		string Save(string file);

		/// <summary>Saves a boot skin to a stream</summary>
		/// <param name="data">The stream to save to</param>
		void Save(System::IO::Stream ^data);

		/// <summary>Saves a boot skin to a file, possibly using the old format</summary>
		/// <param name="file">The file path to save to</param>
		/// <param name="old_format">True if the old format should be used (non-MIME multipart wrapped), false otherwise</param>
		/// <returns>A string containing an error message if the file could not be written, or null if it succeeded to save</returns>
		string Save(string file, bool old_format);

		/// <summary>Saves a boot skin to a stream, possibly using the old format</summary>
		/// <param name="data">The stream to save to</param>
		/// <param name="old_format">True if the old format should be used (non-MIME multipart wrapped), false otherwise</param>
		void Save(System::IO::Stream ^data, bool old_format);

		/// <summary>Gets the boot skin settings for a particular file</summary>
		/// <param name="i">If 0 gets settings for winload. If 1 gets settings for winresume.</param>
		/// <returns>The BootSkinFile holding all the settings for the requested file</returns>
		property BootSkinFile ^WinXXX[unsigned long] { BootSkinFile ^get(unsigned long i); }
		/// <summary>Gets the boot skin settings for Winload</summary>
		/// <returns>The BootSkinFile holding all the settings for Winload</returns>
		property BootSkinFile ^Winload { BootSkinFile ^get(); }
		/// <summary>Gets the boot skin settings for Winresume</summary>
		/// <returns>The BootSkinFile holding all the settings for Winresume</returns>
		property BootSkinFile ^Winresume { BootSkinFile ^get(); }
	};
}
