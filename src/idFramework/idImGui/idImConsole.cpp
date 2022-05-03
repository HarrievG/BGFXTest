#include "idImConsole.h"
#include <stdlib.h>         // NULL, malloc, free, atoi
#include <imgui.h>
#include <type_traits>
#include <tuple>
#include "idFramework/CVarSystem.h"
#include "idFramework/common.h"
#include <idFramework/idlib/containers/StrList.h>

static idImConsole localConsole;
idImConsole *imConsole = &localConsole;

extern idCVar com_timestampPrints;

static autoComplete_t	globalAutoComplete;
static void  Strtrim( char *s ) { char *str_end = s + strlen( s ); while ( str_end > s && str_end[-1] == ' ' ) str_end--; *str_end = 0; }
static idStr itemStr;

struct parms {
    ImVector<const char *> *listptr;
    const char *word_start;
    const char *word_end;
    idCmdArgs *args;
    bool used;
};

idImConsole::idImConsole( )  {
    ClearLog( );
    memset( InputBuf, 0, sizeof( InputBuf ) );
    HistoryPos = -1;

    //Commands.push_back( "HISTORY" );
    //Commands.push_back( "CLEAR" );
	//static cache_t cache;
	//static CacheSetup setup( std::cout, true_callback, &cache );

	//std::ofstream file;
	//file.open( "cout.txt" );
	//std::streambuf *sbuf = std::cout.rdbuf( );
	//std::cout.rdbuf( file.rdbuf( ) );
	//cout is now pointing to a file
}


void idImConsole::Shutdown( ) {
	ClearLog( );
	for ( int i = 0; i < History.Size; i++ )
		free( History[i] );
	History.clear();
	itemStr.Clear();
}

idImConsole::~idImConsole( ) {
	Shutdown();
}

// Portable helpers

int idImConsole::Stricmp( const char *str1, const char *str2 ) { int d; while ( ( d = toupper( *str2 ) - toupper( *str1 ) ) == 0 && *str1 ) { str1++; str2++; } return d; }

int idImConsole::Strnicmp( const char *str1, const char *str2, int n ) { int d = 0; while ( n > 0 && ( d = toupper( *str2 ) - toupper( *str1 ) ) == 0 && *str1 ) { str1++; str2++; n--; } return d; }

char *idImConsole::Strdup( const char *str ) { size_t len = strlen( str ) + 1; void *buff = malloc( len ); return ( char * ) memcpy( buff, ( const void * ) str, len ); }

void idImConsole::ClearLog( )     {
    for ( int i = 0; i < Items.Size; i++ )
        free( Items[i] );
    Items.clear( );
    ScrollToBottom = true;
}

//Do not call directly.
//Use common->Printf instead.
void idImConsole::AddLog(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf)-1] = 0;
    va_end(args);
    Items.push_back(Strdup(buf));
    ScrollToBottom = true;
}

void idImConsole::Draw( )
{
	imDraw("TTY",&open );
}

void idImConsole::imDraw( const char *title, bool *p_open ){
      {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
        // So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;
            ImGui::EndPopup();
        }

        //ImGui::TextWrapped(
        //    "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
        //    "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
        //ImGui::TextWrapped("Enter 'HELP' for help.");

        // TODO: display items starting from the bottom

        //if (ImGui::SmallButton("Add Debug Text"))  { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
        //ImGui::SameLine();
        //if (ImGui::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
        //ImGui::SameLine();
        if (ImGui::SmallButton("Clear")) { 
            ClearLog(); 
        }
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::SmallButton("Copy");
        //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

        ImGui::Separator();
        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Options, Filter
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear")) ClearLog();
            ImGui::EndPopup();
        }

        ////
        /// Clipper is done. pls stream to a file!
        ///
        // Display every line as a separate entry so we can change their color or add custom widgets.
        // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
        // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
        // to only process visible items. The clipper will automatically measure the height of your first item and then
        // "seek" to display only items in the visible area.
        // To use the clipper we can replace your standard loop:
        //      for (int i = 0; i < Items.Size; i++)
        //   With:
        //      ImGuiListClipper clipper;
        //      clipper.Begin(Items.Size);
        //      while (clipper.Step())
        //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        // - That your items are evenly spaced (same height)
        // - That you have cheap random access to your elements (you can access them given their index,
        //   without processing all the ones before)
        // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
        // We would need random-access on the post-filtered list.
        // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
        // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
        // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
        // to improve this example code!
        // If your items are of variable height:
        // - Split them into same height items would be simpler and facilitate random-seeking into your list.
        // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        
        for (int i = 0; i < Items.Size; i++)
        {
            const char *item = Items[i];
            if ( Filter.PassFilter( item ) )
                VisibleItems.push_back(item);
        }

        ImGuiListClipper clipper;
        clipper.Begin( VisibleItems.Size);
        while ( clipper.Step( ) )
            for ( int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++ ){
        //for (int i = 0; i < Items.Size; i++)
        //{
            const char* item = VisibleItems[i];
            //if (!Filter.PassFilter(item))
             //   continue;

			auto colconv = [](idVec4 idcol) -> ImVec4
				{return ImVec4(idcol.x, idcol.y, idcol.z, idcol.w );};

			idVec4 idColor;
			ImVec4 color;
			idVec4 setColor = idStr::ColorForIndex( 7 );
			ImVec4 targetCol = colconv(setColor);
			const unsigned char *s;

			s = ( const unsigned char * ) item;
			bool forceColor = false;
			bool coloring = false;
			uint idx = 0;

			itemStr.Clear();
			while (*s) {
				if (idStr::IsColor( ( const char * ) s ))
				{
					//if we had an colour, flush it.
					if ( coloring )
					{
						ImGui::PushStyleColor( ImGuiCol_Text, color );
						ImGui::TextUnformatted( itemStr );ImGui::SameLine(0, 0);
						ImGui::PopStyleColor( );
						itemStr.Clear();
						coloring = false;
					}
					coloring = true;
					targetCol = colconv( idStr::ColorForIndex( *( s + 1 ) ) );
					if ( !forceColor ) {
						if ( *( s + 1 ) == C_COLOR_DEFAULT ) {
							color = colconv( setColor );
						} else {
							color = targetCol ;
							color.w = setColor[3];
						}
					}
					idx += 2;
					s += 2;
					continue;
				}else
					itemStr+= item[idx];
				s++;
				idx++;
			}
			if ( coloring ) 					{
				ImGui::PushStyleColor( ImGuiCol_Text, color );
				ImGui::TextUnformatted( itemStr );
				ImGui::PopStyleColor( );
			}else if (itemStr.Length())
				ImGui::TextUnformatted( itemStr );
        }
        if (copy_to_clipboard)
            ImGui::LogFinish();

        if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackEdit;
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags,
            []( ImGuiInputTextCallbackData *data ) -> int {
                idImConsole *console = ( idImConsole * ) data->UserData;
                return console->TextEditCallback( data );
            }, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
    }
    VisibleItems.clear();
}

void idImConsole::ExecCommand( const char *command_line )     {
    common->Printf( "^8# %s\n", command_line);
	// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
	HistoryPos = -1;
	for ( int i = History.Size - 1; i >= 0; i-- )
		if ( Stricmp( History[i], command_line ) == 0 ) {
			free( History[i] );
			History.erase( History.begin( ) + i );
			break;
		}
	History.push_back( Strdup( command_line ) );
	// Process command
	if ( Stricmp( command_line, "CLEAR" ) == 0 ) {
		ClearLog( );
	} else if ( Stricmp( command_line, "HISTORY" ) == 0 ) {
		for ( int i = History.Size >= 10 ? History.Size - 10 : 0; i < History.Size; i++ )
			AddLog( "%3d: %s\n", i, History[i] );
	} else {
        cmdSystem->BufferCommandText( CMD_EXEC_APPEND, command_line );	// valid command
        cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "\n" );
	}
}

int idImConsole::TextEditCallback(ImGuiInputTextCallbackData* data)
{
    static int lastBuffLenght = -1;

    //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion:
        {
            char completionArgString[MAX_EDIT_LINE];
            idCmdArgs args;
            args.TokenizeString( data->Buf, false );
             
            autoComplete.valid &= args.Argc() > 0;
            if ( autoComplete.valid && args.Argc() >= 1 )
            {
                //autoComplete.valid &= abs(idStr::Length( args.Argv( args.Argc() - 1 ) ) - autoComplete.length) <= 1;
                autoComplete.valid &= autoComplete.valid && autoComplete.completionString[autoComplete.length] == '\0';
                if (args.Argc( ) == 1)
                    autoComplete.valid &= lastBuffLenght <= data->BufTextLen;
            }
            lastBuffLenght = data->BufTextLen;

            if ( !autoComplete.valid ) 
            {
                idStr::Copynz( autoComplete.completionString, args.Argv( 0 ), sizeof( autoComplete.completionString ) );
                idStr::Copynz( completionArgString, args.Args( ), sizeof( completionArgString ) );
                autoComplete.matchCount = 0;
                autoComplete.matchIndex = 0;
                autoComplete.currentMatch[0] = 0;
            }
            if ( strlen( autoComplete.completionString ) == 0 ) 
                return 0;

            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            int wordCnt = args.Argc() - 1;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                word_start--;
            }

            ImVector<const char *> candidates;
            parms parm = { &candidates,word_start,word_end,&args,false };
            static parms * parmPtr = nullptr;
            parmPtr = &parm;
            //arg completion
            if ( autoComplete.valid )
            {
                auto find = []( const char *cmd ) -> auto {
                    const char *start = cmd;
                    start += 1 + strlen( parmPtr->args->Argv( 0 ) );
                    if ( Strnicmp( start, parmPtr->word_start, ( int ) ( parmPtr->word_end - parmPtr->word_start ) ) == 0 )
                        parmPtr->listptr->push_back( cmd + 1 + strlen( parmPtr->args->Argv( 0 ) ) );
                    parmPtr->used = true;
                };
                cmdSystem->ArgCompletion( args.Args(0,-1), find );
                cvarSystem->ArgCompletion( args.Args(0,-1), find );
                autoComplete.matchCount = candidates.Size;
            }else if (word_start != word_end)
            {
                // Build a list of candidates
                bool argUsed = false;
			    cvarSystem->CommandCompletion( [](const char * cmd) -> auto {			
				    if (Strnicmp(cmd, parmPtr->word_start, (int)( parmPtr->word_end - parmPtr->word_start)) == 0)
					    parmPtr->listptr->push_back( cmd );
				    });

			    cmdSystem->CommandCompletion( []( const char *cmd ) -> auto {
				    if ( Strnicmp( cmd, parmPtr->word_start, (int) ( parmPtr->word_end - parmPtr->word_start)) == 0 )
					    parmPtr->listptr->push_back( cmd );
				    });

                autoComplete.matchCount = candidates.Size;
            }
            if (candidates.Size == 0)
            {
                // No match
                AddLog("^3No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
            }
            else if (candidates.Size == 1 )
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
                autoComplete.length = strlen( data->Buf );
                autoComplete.valid = true;
            }
            else
            {
                // Multiple matches. Complete as much as we can..
                // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                int match_len = (int)(word_end - word_start);
                if (match_len)
                for (;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                    {
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    }
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }
                
                if (match_len > 1 )
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars((int)(word_start - data->Buf), candidates[0], candidates[0] + match_len );
                }
                
                // List matches
                common->Printf("Possible matches:\n");
                for (int i = 0; i < candidates.Size; i++)
                    common->Printf(" %i)- %s\n", i+1,candidates[i]);
            }
            idStr::snPrintf( autoComplete.completionString, idStr::Length( data->Buf ) + 1, "%s", data->Buf );
            autoComplete.length = strlen( autoComplete.completionString );
                
            
            break;
        }
    case ImGuiInputTextFlags_CallbackHistory:
        {
            const int prev_history_pos = HistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (HistoryPos == -1)
                    HistoryPos = History.Size - 1;
                else if (HistoryPos > 0)
                    HistoryPos--;
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (HistoryPos != -1)
                    if (++HistoryPos >= History.Size)
                        HistoryPos = -1;
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != HistoryPos)
            {
                const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
    }
    return 0;
}
