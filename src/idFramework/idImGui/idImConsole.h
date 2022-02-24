#ifndef __IDIMCONSOLE_H__
#define __IDIMCONSOLE_H__
#include <ctype.h>          // toupper, isprint
#include "imgui.h"
#include <stdio.h>

const int MAX_EDIT_LINE = 256;
typedef struct autoComplete_s {
	bool			valid;
	int				length;
	char			completionString[MAX_EDIT_LINE];
	char			currentMatch[MAX_EDIT_LINE];
	int				matchCount;
	int				matchIndex;
	int				findMatchIndex;
} autoComplete_t;


class idImConsole {
public:
	void						ClearLog( );
	char						InputBuf[256];

	idImConsole( );
	~idImConsole( );
	void Shutdown();
	// Portable helpers
	static int					Stricmp( const char *str1, const char *str2 );
	static int					Strnicmp( const char *str1, const char *str2, int n );
	static char	*				Strdup( const char *str );

	void						AddLog( const char *fmt, ... );
	void						Draw();
	void						ExecCommand( const char *command_line );
	int							TextEditCallback( ImGuiInputTextCallbackData *data );
private:

	void						imDraw( const char *title, bool *p_open );
	ImVector<const char *>			VisibleItems;
	ImVector<char *>			Items;
	ImVector<char *>			History;
	int							HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter				Filter;
	bool						AutoScroll;
	bool						ScrollToBottom;
	bool						open;
	autoComplete_t				autoComplete;
};

extern idImConsole * imConsole;
#endif /* !__IDIMCONSOLE_H__ */
