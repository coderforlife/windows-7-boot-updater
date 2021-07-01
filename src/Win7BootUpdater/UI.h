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

#ifdef ERROR
#undef ERROR
#endif

namespace Win7BootUpdater {
	/// <remarks>All of the messages that are available for localization.</remarks>
	PUBLIC enum struct Msg abstract sealed : int {
		// Core
		NO_MESSAGE = 0,
		Translator,
		FailedToCreateABackupOf,
		FileCouldNotBeReadAsPE,
		FileDoesNotContainProperResources,
		FileIsNotAccordingToItsVersionInfo,
		FileLooksLike,
		FailedToUpdateTheWIMResource,
		FailedToUpdateTheResources,
		FailedToRemoveTheDigitalCertificate,
		FailedToSaveTheUpdated,
		FailedToDeactiveFilesystemSecurity,
		FailedToRestoreFilesystemSecurity,
		FailedToUpdateTheMessageTableText,
		FailedToSaveTheAnimation,
		FailedToCaptureTheWIM,
		FailedToReadTheWIM,
		FailedToUpdate,
		FailedToUpdateTheBootFont,
		FailedToUpdateTheCopyright,
		FailedToUpdateTheTextAndBackground,
		FailedToDisableBootresWinloadSecurity,
		FailedToDisableBootmgrSecurity,
		FailedToRecompessBootmgr,
		FailedToCreateATemporaryFile,
		FailedToCreateATemporaryDirectory,
		FailedToDeleteATemporaryFile,
		ThereWasAnUncaughtExceptionWhileUpdatingTheFiles,
		UnknownError,
		CreateFileFailed,
		ErrorWhileGettingPath,
		UnableToLoadTheBCDStore,
		PathInTheBCDIsEitherInvalidOrNotYetSupported,
		PathInTheBCDIsInvalid,
		PathInTheBCDReferencesAnEFIWhichIsCurrentlyNotSupported,
		TheBootSkinIsNotAnUnderstoodVersion,
		TheAnimationSourceCannotBeWinloadForWinload,
		TheFileCouldNotBeOpenedForWriting,
		TheFileCouldNotBeValidated,
		TheFileCouldNotBeRead,
		ErrorLoadingBootSkin,
		FailedToGetBootSkinData,
		Installer,
		LoadingPatchFailed,
		ThePatchVersionIsNewerThanThisProgramCanUnderstand,
		PatchOutOfDate,
		CouldNotOpenTheFileAsAnImageFile,
		TheDirectoryCouldNotBeOpened,
		TheDirectoryDoesNotContainAnyAcceptableImageFiles,
		ItMustBeAFullActivityBmpOr105Frames,
		OpeningAnimationError,
		Updating,
		FinishingUp,
		CompressingAnimation,
		ModifyingResources,
		NoError,
		Windows7BootUpdater,
		Y, y, N, n,
		ERROR,

		// General UI - Errors
		FailedToEnableTheTakeOwnershipPrivilege,
		CriticalError,
		LoadError,
		SaveError,
		UnrecognizedOption,
		ErrorWith,
		ThereWasAProblemVerifying,
		SelectADifferentFile,
		GoToOptionsAndSelectAnAppropiate,
		WrongNumberOfArguments,
		CouldNotFindTheGivenBS7File,
		TheBS7FileProvidedIsInvalid,
		ThereWasAnUncaughtExcpetionWhileUpdatingTheFiles,
		ThereWasAProblemUpdatingTheFiles,
		BootUpdateException,
		BootUpdateError,
		DoYouWishToSaveTheCurrentSettingsBeforeLoadingADifferentBootSkin,
		SaveModifications,
		ThereWasAProblemCreatingTheInstaller,
		CreateInstallerError,
		UnknownMCIError,
		FailedToOpenLink,

		// General UI - Messages and Dialogs
		ComingSoon,
		TheFollowingFilesWereFound,
		NoAcceptableFilesWereFound,
		SelectWindowsFolder,
		RestoredTo,
		NoFilesWereRestored,
		FileRestoration,
		SuccessfullyUpdatedTheBootAnimationAndText,
		BootUpdateSuccess,
		About,
		UploadBootSkin,
		VersionCheck,
		YouAreRunningTheLatestVersion,
		YouAreNotRunningTheLatestVersion,
		SelectBackgroundImage,
		SelectTheFolderThatContainsTheAnimation,
		AnimationInformation,
		Close,
		ImageFiles,
		AllFiles,
		BootSkinForWindows7,
		FolderOpt,
		FileDefault,
		OnHiddenSystemPartition,

		// Command Line
		CommandLineVersion,
		Usage,
		Options,
		OrToRestoreBackupFiles,
		WhereTheOptionsAre,
		SetsAsManyOfTheOptionsBelowAsPossible,
		YouCanUseTheGUIProgramToCreateBS7Files,

		// General UI - About
		Version,
		ByJeffBush,
		VisitForMoreInformation,
		SeeForLicensingOfThisProduct,
		ProblemsBugsCommentsEmailMeAt,
		MyHomepage,
		Windows7BootPage,
		LikeThisProgramDonate,
		ThanksTo,
		ForAllOfTheirInput,
		ForReferencePNGLibrary,
		ForTheMonitorPaintingLogo,
		AndAllThePeopleWhoTestedTheProgramAndGaveWonderfulFeedback,
		Translations,
		TranslatedBy,
		Versions,
		Database,
		License,

		// GUI - Menus
		Colon,
		File,
		LoadBootSkin,
		LoadBootSkinFromSystemFiles,
		SaveBootSkinAs,
		CreateBootSkinInstaller,
		UploadBootSkin_Menu,
		BrowsePublicBootSkins,
		Exit,
		Options_Menu,
		SelectWindowsFolder_Menu,
		RestoreBackups,
		Language,
		ContributeATranslation,
		Help,
		About_Menu,
		CheckForUpdates,
		Windows7BootUpdaterWebpage,
		Donate_Menu,

		// GUI - Editor
		Booting,
		Resuming,
		Animation,
		Same,
		Default,
		StaticImage,
		Animation_Key,
		Background_Key,
		Method,
		Simple,
		Complete,
		Messages,
		Count,
		None,
		Message_1,
		Messages_2,
		Message,
		Text,
		FontColor,
		Position,
		FontSize,
		Background,
		Apply,
		Straight,
		Loop,
		Fullscreen,
		Pause,
		Play,
		Frame,
		Embedded,
		NoneSelected,

		// GUI - Fullscreen control
		ESC,
		Space,
		LeftRight,
		HomeEnd,
		ExitFullscreen,
		PlayPause,
		BackForward1Frame,
		FirstLastFrame,

		// GUI - Create Installer
		CreateInstaller,
		CreateInstallerDesc,
		SkinName,
		SkinAuthor,
		URL,
		Description,
		License_,
		Create,
		YouMustProvideANameAndAuthorForTheSkin,

		// GUI - reCaptcha
		PlayAgain,
		GetAVisualChallenge,
		GetAnAudioChallenge,
		GeANewChallenge,

		// Installer
		Windows7BootSkin,
		Windows7BootSkinInstaller,
		BootSkinInstallerForWindows7,
		ThisCanOnlyBeUsedInWindows7,
		by,
		FailedToRestore,
		Back,
		Next,
		Cancel,
		IAgree,
		Install,
		LicenseAgreement,
		PleaseReviewTheLicenseTermsBeforeInstallingTheBootSkin,
		UninstallCurrent,
		ReadyToInstall,
		Installing,
		SuccessfullyInstalled,
		FailedToInstall,
		ReadyToUninstall,
		Uninstalling,
		SuccessfullyUninstalled,
		FailedToUninstall,
		TheWindows7BootUpdaterTerms,
		TheTermsForTheBootSkinBy,
		IfYouAcceptTheTermsOfTheAgreement,
		Welcome_Full,
		AnotherBootSkinIsAlreadyInstalled,
		ReadyToInstall_Full,
		Installing_Full,
		SuccessfullyInstalled_Full,
		FailedToInstall_Full,
		ReadyToUninstall_Full,
		Uninstalling_Full,
		SuccessfullyUninstalled_Full,
		FailedToUninstall_Full,
	};

	/// <remarks>
	/// A static class that contains many methods and events for interacting with a user interface.
	/// Essentially the only way the core communicates with any interface is through this class.
	/// The ErrorMessenger, Messenger, YesNoMessenger, and ProgressChanged should all be defined before doing anything with the core so messages can be received.
	/// This class additionally provides localization of messages, many which are not used by the core but are intended for use by the interface itself.
	/// </remarks>
	PUBLIC ref class UI abstract sealed {
	public:
		/// <summary>
		/// The locale that is being used for <see cref="GetMessage" />.
		/// Any locale will be set and the best possible choice will be used.
		/// The default is the closest match to the system locale.
		/// A specific locale may not be fully translated and English messages will be used if necessary.
		/// </summary>
		static property System::Globalization::CultureInfo ^Locale { System::Globalization::CultureInfo ^get(); void set(System::Globalization::CultureInfo^); }
		/// <summary>Gets a list of all translations available</summary>
		/// <returns>A list of all locales for which at least one message has been translated</returns>
		static array<System::Globalization::CultureInfo^> ^AvailableLocales();
		/// <summary>Gets the name that should be displayed for a specific locale</summary>
		/// <param name="l">The locale to get the name for</param>
		/// <returns>The displayable name, in the native language of the locale</returns>
		static string LocaleDisplayName(System::Globalization::CultureInfo ^l);
		/// <summary>Gets a list of all the "TranslatedBy" messages provided in the messages</summary>
		/// <returns>A list of all translators, it is the same length as AvailableLocales() and lines up with those values</returns>
		static array<string> ^GetTranslators();

		/// <summary>Gets and formats a localized message based on the current locale, defaulting to English if necessary</summary>
		/// <param name="m">The message to get</param>
		/// <param name="opts">If the message requires formatting options (because it uses {0}) then these options are passed to the formatter</param>
		/// <returns>The formatted localized message</returns>
		static string GetMessage(Msg m, ... array<object> ^opts);
		/// <summary>Gets an error message string based on an error code</summary>
		/// <param name="e">The error code</param>
		/// <param name="success">The string to return if the error code signifies success</param>
		static string GetErrorMessage(uint e, string success);

#pragma warning(push)
#pragma warning(disable:4693)
		/// <remarks>The delegate for <see cref="ErrorMessenger" /> and <see cref="Messenger" />.</remarks>
		delegate void Message(string text, string caption);
		/*
		/// <remarks>The delegate for <see cref="YesNoMessenger" />.</remarks>
		delegate bool YesNoMsg(string text, string caption);
		*/
		/// <remarks>The delegate for the <see cref="ProgressChanged" /> event.</remarks>
		delegate void Progress(string text, int cur, int max);
#pragma warning(pop)

		/// <summary>The <see cref="Message" /> that is called whenever an error needs to be reported.</summary>
		static Message ^ErrorMessenger;
		/// <summary>Shows an error message to the user</summary>
		/// <param name="text">The content of the message</param>
		/// <param name="caption">The caption of the message</param>
		static void ShowError(string text, string caption);
		/// <summary>Shows an error message to the user</summary>
		/// <param name="text">The content of the message, as a <see cref="Msg" /> code</param>
		/// <param name="opt">An option passed to format the text <see cref="Msg" /></param>
		/// <param name="caption">The caption of the message, as a <see cref="Msg" /> code</param>
		static void ShowError(Msg text, string opt, Msg caption);

		/// <summary>Shows an error message to the user based on an error code</summary>
		/// <param name="pre">The text that shows before the error text</param>
		/// <param name="post">The text that shows after the error text</param>
		/// <param name="err">The error code</param>
		/// <param name="caption">The caption of the message</param>
		static void ShowError(string pre, string post, uint err, string caption);

		/*
		/// <summary>The <see cref="Message" /> that is called whenever a message needs to be reported.</summary>
		static Message ^Messenger;
		/// <summary>Shows a message to the user</summary>
		/// <param name="text">The content of the message</param>
		/// <param name="caption">The caption of the message</param>
		static void ShowMessage(string text, string caption);
		//static void ShowMessage(string text, Msg caption);
		//static void ShowMessage(Msg text, Msg caption);
		*/

		/*
		/// <summary>The <see cref="YesNoMsg" /> that is called whenever a Yes/No question needs to be asked to the user, returns true if the user clicks Yes.</summary>
		static YesNoMsg ^YesNoMessenger;
		/// <summary>Asks a Yes/No question to the user</summary>
		/// <param name="text">The content of the message</param>
		/// <param name="caption">The caption of the message</param>
		/// <returns>True if the user clicks yes, false otherwise</returns>
		static bool AskYesNo(string text, string caption);
		*/

		/// <remarks>The event progress notifications.</remarks>
		static event Progress ^ProgressChanged;
		/// <summary>Initializes the progress bar</summary>
		/// <param name="max">The maximum value to use in the progress bar</param>
		static void InitProgress(int max);
	internal:
		static property int ProgressMax { int get(); void set(int x); }
		static property int ProgressCurrent { int get(); void set(int x); }
		static property string ProgressText { string get(); void set(string x); }
		static int Inc();
		static int Inc(int x);
		static int Inc(string text);

	private:
		static void Init();

		static void LoadLanguage(string name, System::Collections::Generic::List<string> ^list);

		static System::Collections::Generic::Dictionary<System::Globalization::CultureInfo^, string> ^localeToNames = nullptr;
		static array<System::Globalization::CultureInfo^> ^locales = nullptr;
		static System::Globalization::CultureInfo ^locale = nullptr;

		static System::Collections::Generic::List<string> ^messages = nullptr, ^fallbackMsgs = nullptr, ^englishMsgs = nullptr;
		static array<string> ^translators = nullptr;

		static string text;
		static int cur, max;
		static void OnChange();
	};
}
