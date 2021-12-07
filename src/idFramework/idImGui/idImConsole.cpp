#include "idImConsole.h"
#include <stdlib.h>         // NULL, malloc, free, atoi
#include <imgui.h>
#include <type_traits>
#include <tuple>
#include "idFramework/CVarSystem.h"


static idImConsole localConsole;
idImConsole *imConsole = &localConsole;

static void  Strtrim( char *s ) { char *str_end = s + strlen( s ); while ( str_end > s && str_end[-1] == ' ' ) str_end--; *str_end = 0; }

template <typename T>
struct function_traits
    : public function_traits<decltype( &T::operator() )> { };

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType( ClassType:: * )( Args... ) const> {
    template <size_t i>
    using arg_t = std::tuple_element_t<i, std::tuple<Args...>>;
};
//
//
//struct test {
//    test( ImGuiInputTextCallbackData *data ) { };
//    idImConsole *ptr;
//};
//test tt( nullptr );
//tt.ptr = this;
//test *ptrTT = &tt;
//
////idImConsole * replacementChar = this;
//auto replacementLambda = [&ptrTT] ( ImGuiInputTextCallbackData *data ) {
//    data->UserData = ( void * ) ptrTT;
//    return 0;
//};
//using ReplacementLambdaType = decltype( tt );
//if ( ImGui::InputText( "Input", InputBuf, IM_ARRAYSIZE( InputBuf ), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
//    [] ( ImGuiInputTextCallbackData *data ) {
//        auto f = *static_cast< ReplacementLambdaType * >( data->UserData );
//        f.ptr->ClearLog( );
//        return 0;
//    }, &replacementLambda )
//    )
//idImConsole::idImConsole( )  {
//    ClearLog( );
//    memset( InputBuf, 0, sizeof( InputBuf ) );
//    HistoryPos = -1;
//    Commands.push_back( "HELP" );
//    Commands.push_back( "HISTORY" );
//    Commands.push_back( "CLEAR" );
//    Commands.push_back( "CLASSIFY" );  // "classify" is here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
//    AddLog( "Welcome to ImGui!" );
//}
//
//idImConsole::~idImConsole( )     {
//    ClearLog( );
//    for ( int i = 0; i < History.Size; i++ )
//        free( History[i] );
//}

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

        ImGui::TextWrapped(
            "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
            "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
        ImGui::TextWrapped("Enter 'HELP' for help.");

        // TODO: display items starting from the bottom

        if (ImGui::SmallButton("Add Debug Text"))  { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
        ImGui::SameLine();
        if (ImGui::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear"))           { ClearLog(); }
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
            const char* item = Items[i];
            if (!Filter.PassFilter(item))
                continue;

            // Normally you would store more information in your item than just a string.
            // (e.g. make Items[] an array of structure, store color/type etc.)
            ImVec4 color;
            bool has_color = false;
            if (strstr(item, "[error]"))          { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
            else if ( strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
            if (has_color)
                ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(item);
            if (has_color)
                ImGui::PopStyleColor();
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
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
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
}

void idImConsole::ExecCommand( const char *command_line )     {
	AddLog( "# %s\n", command_line );
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
	} else if ( Stricmp( command_line, "HELP" ) == 0 ) {
		AddLog( "Commands:" );
		for ( int i = 0; i < Commands.Size; i++ )
			AddLog( "- %s", Commands[i] );
	} else if ( Stricmp( command_line, "HISTORY" ) == 0 ) {
		for ( int i = History.Size >= 10 ? History.Size - 10 : 0; i < History.Size; i++ )
			AddLog( "%3d: %s\n", i, History[i] );
	} else {
		AddLog( "Unknown command: '%s'\n", command_line );
	}
}

int idImConsole::TextEditCallback(ImGuiInputTextCallbackData* data)
{
    //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Example of TEXT COMPLETION

            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                word_start--;
            }

			struct parms {
				ImVector<const char *>* listptr;
				const char * word_start;
				const char * word_end;
			};
			
            // Build a list of candidates
            ImVector<const char*> candidates;
            for (int i = 0; i < Commands.Size; i++)
                if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                    candidates.push_back(Commands[i]);

			parms parm = { &candidates,word_start,word_end };
			cvarSystem->CommandCompletion( [](const char * cmd,void * usr) -> auto {			
				//((ImVector<const char *>*)usr)->push_back( cmd );
				parms * lparms = (parms*)usr;
				if (Strnicmp(cmd, lparms->word_start, (int)( lparms->word_end - lparms->word_start)) == 0)
					lparms->listptr->push_back( cmd );
				}, &parm );
			cmdSystem->CommandCompletion( []( const char *cmd, void *usr ) -> auto {
				parms *lparms = ( parms * ) usr;
				if ( Strnicmp( cmd, lparms->word_start, ( int ) ( lparms->word_end - lparms->word_start ) ) == 0 )
					lparms->listptr->push_back( cmd );
				//( ( ImVector<const char *> * )usr )->push_back( cmd );
			}, &parm );

            if (candidates.Size == 0)
            {
                // No match
                AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
            }
            else if (candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can..
                // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                int match_len = (int)(word_end - word_start);
                for (;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }

                // List matches
                AddLog("Possible matches:\n");
                for (int i = 0; i < candidates.Size; i++)
                    AddLog("- %s\n", candidates[i]);
            }

            break;
        }
    case ImGuiInputTextFlags_CallbackHistory:
        {
            // Example of HISTORY
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
