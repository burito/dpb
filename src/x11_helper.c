
#include <X11/X.h>
#include <X11/Xproto.h>
#include <stddef.h>

char* x11_error(unsigned long result)
{
	switch( result ) {
	case Success:			// 0
		return "Success";
	case BadRequest:		// 1
		return "BadRequest";
	case BadValue:			// 2
		return "BadValue";
	case BadWindow:			// 3
		return "BadWindow";
	case BadPixmap:			// 4
		return "BadPixmap";
	case BadAtom:			// 5
		return "BadAtom";
	case BadCursor:			// 6
		return "BadCursor";
	case BadFont:			// 7
		return "BadFont";
	case BadMatch:			// 8
		return "BadMatch";
	case BadDrawable:		// 9
		return "BadDrawable";
	case BadAccess:			// 10
		return "BadAccess";
	case BadAlloc:			// 11
		return "BadAlloc";
	case BadColor:			// 12
		return "BadColor";
	case BadGC:			// 13
		return "BadGC";
	case BadIDChoice:		// 14
		return "BadIDChoice";
	case BadName:			// 15
		return "BadName";
	case BadLength:			// 16
		return "BadLength";
	case BadImplementation:		// 17
		return "BadImplementation";
	default:
		return NULL;
	}
}


char* x11_opcode(unsigned long opcode)
{
	switch( opcode ) {
	case X_CreateWindow:			// 1
		return "X_CreateWindow";
	case X_ChangeWindowAttributes:		// 2
		return "X_ChangeWindowAttributes";
	case X_GetWindowAttributes:		// 3
		return "X_GetWindowAttributes";
	case X_DestroyWindow:			// 4
		return "X_DestroyWindow";
	case X_DestroySubwindows:		// 5
		return "X_DestroySubwindows";
	case X_ChangeSaveSet:			// 6
		return "X_ChangeSaveSet";
	case X_ReparentWindow:			// 7
		return "X_ReparentWindow";
	case X_MapWindow:			// 8
		return "X_MapWindow";
	case X_MapSubwindows:			// 9
		return "X_MapSubwindows";
	case X_UnmapWindow:			// 10
		return "X_UnmapWindow";
	case X_UnmapSubwindows:			// 11
		return "X_UnmapSubwindows";
	case X_ConfigureWindow:			// 12
		return "X_ConfigureWindow";
	case X_CirculateWindow:			// 13
		return "X_CirculateWindow";
	case X_GetGeometry:			// 14
		return "X_GetGeometry";
	case X_QueryTree:			// 15
		return "X_QueryTree";
	case X_InternAtom:			// 16
		return "X_InternAtom";
	case X_GetAtomName:			// 17
		return "X_GetAtomName";
	case X_ChangeProperty:			// 18
		return "X_ChangeProperty";
	case X_DeleteProperty:			// 19
		return "X_DeleteProperty";
	case X_GetProperty:			// 20
		return "X_GetProperty";
	case X_ListProperties:			// 21
		return "X_ListProperties";
	case X_SetSelectionOwner:		// 22
		return "X_SetSelectionOwner";
	case X_GetSelectionOwner:		// 23
		return "X_GetSelectionOwner";
	case X_ConvertSelection:		// 24
		return "X_ConvertSelection";
	case X_SendEvent:			// 25
		return "X_SendEvent";
	case X_GrabPointer:			// 26
		return "X_GrabPointer";
	case X_UngrabPointer:			// 27
		return "X_UngrabPointer";
	case X_GrabButton:			// 28
		return "X_GrabButton";
	case X_UngrabButton:			// 29
		return "X_UngrabButton";
	case X_ChangeActivePointerGrab:		// 30
		return "X_ChangeActivePointerGrab";
	case X_GrabKeyboard:			// 31
		return "X_GrabKeyboard";
	case X_UngrabKeyboard:			// 32
		return "X_UngrabKeyboard";
	case X_GrabKey:				// 33
		return "X_GrabKey";
	case X_UngrabKey:			// 34
		return "X_UngrabKey";
	case X_AllowEvents:			// 35
		return "X_AllowEvents";
	case X_GrabServer:			// 36
		return "X_GrabServer";
	case X_UngrabServer:			// 37
		return "X_UngrabServer";
	case X_QueryPointer:			// 38
		return "X_QueryPointer";
	case X_GetMotionEvents:			// 39
		return "X_GetMotionEvents";
	case X_TranslateCoords:			// 40
		return "X_TranslateCoords";
	case X_WarpPointer:			// 41
		return "X_WarpPointer";
	case X_SetInputFocus:			// 42
		return "X_SetInputFocus";
	case X_GetInputFocus:			// 43
		return "X_GetInputFocus";
	case X_QueryKeymap:			// 44
		return "X_QueryKeymap";
	case X_OpenFont:			// 45
		return "X_OpenFont";
	case X_CloseFont:			// 46
		return "X_CloseFont";
	case X_QueryFont:			// 47
		return "X_QueryFont";
	case X_QueryTextExtents:		// 48
		return "X_QueryTextExtents";
	case X_ListFonts:			// 49
		return "X_ListFonts";
	case X_ListFontsWithInfo:		// 50
		return "X_ListFontsWithInfo";
	case X_SetFontPath:			// 51
		return "X_SetFontPath";
	case X_GetFontPath:			// 52
		return "X_GetFontPath";
	case X_CreatePixmap:			// 53
		return "X_CreatePixmap";
	case X_FreePixmap:			// 54
		return "X_FreePixmap";
	case X_CreateGC:			// 55
		return "X_CreateGC";
	case X_ChangeGC:			// 56
		return "X_ChangeGC";
	case X_CopyGC:				// 57
		return "X_CopyGC";
	case X_SetDashes:			// 58
		return "X_SetDashes";
	case X_SetClipRectangles:		// 59
		return "X_SetClipRectangles";
	case X_FreeGC:				// 60
		return "X_FreeGC";
	case X_ClearArea:			// 61
		return "X_ClearArea";
	case X_CopyArea:			// 62
		return "X_CopyArea";
	case X_CopyPlane:			// 63
		return "X_CopyPlane";
	case X_PolyPoint:			// 64
		return "X_PolyPoint";
	case X_PolyLine:			// 65
		return "X_PolyLine";
	case X_PolySegment:			// 66
		return "X_PolySegment";
	case X_PolyRectangle:			// 67
		return "X_PolyRectangle";
	case X_PolyArc:				// 68
		return "X_PolyArc";
	case X_FillPoly:			// 69
		return "X_FillPoly";
	case X_PolyFillRectangle:		// 70
		return "X_PolyFillRectangle";
	case X_PolyFillArc:			// 71
		return "X_PolyFillArc";
	case X_PutImage:			// 72
		return "X_PutImage";
	case X_GetImage:			// 73
		return "X_GetImage";
	case X_PolyText8:			// 74
		return "X_PolyText8";
	case X_PolyText16:			// 75
		return "X_PolyText16";
	case X_ImageText8:			// 76
		return "X_ImageText8";
	case X_ImageText16:			// 77
		return "X_ImageText16";
	case X_CreateColormap:			// 78
		return "X_CreateColormap";
	case X_FreeColormap:			// 79
		return "X_FreeColormap";
	case X_CopyColormapAndFree:		// 80
		return "X_CopyColormapAndFree";
	case X_InstallColormap:			// 81
		return "X_InstallColormap";
	case X_UninstallColormap:		// 82
		return "X_UninstallColormap";
	case X_ListInstalledColormaps:		// 83
		return "X_ListInstalledColormaps";
	case X_AllocColor:			// 84
		return "X_AllocColor";
	case X_AllocNamedColor:			// 85
		return "X_AllocNamedColor";
	case X_AllocColorCells:			// 86
		return "X_AllocColorCells";
	case X_AllocColorPlanes:		// 87
		return "X_AllocColorPlanes";
	case X_FreeColors:			// 88
		return "X_FreeColors";
	case X_StoreColors:			// 89
		return "X_StoreColors";
	case X_StoreNamedColor:			// 90
		return "X_StoreNamedColor";
	case X_QueryColors:			// 91
		return "X_QueryColors";
	case X_LookupColor:			// 92
		return "X_LookupColor";
	case X_CreateCursor:			// 93
		return "X_CreateCursor";
	case X_CreateGlyphCursor:		// 94
		return "X_CreateGlyphCursor";
	case X_FreeCursor:			// 95
		return "X_FreeCursor";
	case X_RecolorCursor:			// 96
		return "X_RecolorCursor";
	case X_QueryBestSize:			// 97
		return "X_QueryBestSize";
	case X_QueryExtension:			// 98
		return "X_QueryExtension";
	case X_ListExtensions:			// 99
		return "X_ListExtensions";
	case X_ChangeKeyboardMapping:		// 100
		return "X_ChangeKeyboardMapping";
	case X_GetKeyboardMapping:		// 101
		return "X_GetKeyboardMapping";
	case X_ChangeKeyboardControl:		// 102
		return "X_ChangeKeyboardControl";
	case X_GetKeyboardControl:		// 103
		return "X_GetKeyboardControl";
	case X_Bell:				// 104
		return "X_Bell";
	case X_ChangePointerControl:		// 105
		return "X_ChangePointerControl";
	case X_GetPointerControl:		// 106
		return "X_GetPointerControl";
	case X_SetScreenSaver:			// 107
		return "X_SetScreenSaver";
	case X_GetScreenSaver:			// 108
		return "X_GetScreenSaver";
	case X_ChangeHosts:			// 109
		return "X_ChangeHosts";
	case X_ListHosts:			// 110
		return "X_ListHosts";
	case X_SetAccessControl:		// 111
		return "X_SetAccessControl";
	case X_SetCloseDownMode:		// 112
		return "X_SetCloseDownMode";
	case X_KillClient:			// 113
		return "X_KillClient";
	case X_RotateProperties:		// 114
		return "X_RotateProperties";
	case X_ForceScreenSaver:		// 115
		return "X_ForceScreenSaver";
	case X_SetPointerMapping:		// 116
		return "X_SetPointerMapping";
	case X_GetPointerMapping:		// 117
		return "X_GetPointerMapping";
	case X_SetModifierMapping:		// 118
		return "X_SetModifierMapping";
	case X_GetModifierMapping:		// 119
		return "X_GetModifierMapping";
	case X_NoOperation:			// 127
		return "X_NoOperation";
	default:
		return NULL;
	}
}