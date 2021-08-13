#include "md.h"
#include "md.c"

typedef struct SiteInfo SiteInfo;
struct SiteInfo
{
    MD_Node *title;
    MD_Node *desc;
    MD_Node *canonical_url;
    MD_Node *author;
    MD_Node *twitter_handle;
    MD_Node *link_dictionary;
    MD_Node *header;
    MD_Node *footer;
    MD_Node *style;
};

typedef struct PageInfo PageInfo;
struct PageInfo
{
    MD_Node *title;
    MD_Node *desc;
    MD_Node *date;
    MD_Node *parent;
    MD_Node *header_image;
};

static PageInfo ParsePageInfo(MD_Node *page);
static SiteInfo ParseSiteInfo(MD_Node *site);
static MD_String8 MakeDateString(MD_Node *date);
static void GeneratePageContent(MD_Map *index_table, SiteInfo *site_info, PageInfo *page_info, FILE *file, MD_Node *node);

static MD_Arena *arena = 0;

int main(int argument_count, char **arguments)
{
    MD_ThreadContext tctx;
    MD_ThreadInit(&tctx);
    
    arena = MD_ArenaAlloc(1ull << 40);
    
    //~ NOTE(rjf): Parse command line arguments.
    MD_String8List arg_list = MD_StringListFromArgCV(arena, argument_count, arguments);
    MD_CmdLine cmdln = MD_MakeCmdLineFromOptions(arena, arg_list);
    
    MD_String8List siteinfo_values = MD_CmdLineValuesFromString(cmdln, MD_S8Lit("siteinfo"));
    MD_String8 site_info_path = MD_S8ListJoin(arena, siteinfo_values, 0);
    
    MD_String8List pagedir_values = MD_CmdLineValuesFromString(cmdln, MD_S8Lit("pagedir"));
    MD_String8 page_dir_path = MD_S8ListJoin(arena, pagedir_values, 0);
    
    if(!MD_CmdLineB32FromString(cmdln, MD_S8Lit("siteinfo")) ||
       !MD_CmdLineB32FromString(cmdln, MD_S8Lit("pagedir")))
    {
        fprintf(stderr, "USAGE: %s --siteinfo <path to site info file> --pagedir <path to directory with pages> ...\n", arguments[0]);
        goto end;
    }
    
    //~ NOTE(rjf): Load JS.
    MD_String8 js_path = MD_S8Fmt(arena, "%.*s/site.js", MD_S8VArg(page_dir_path));
    MD_String8 js_string = MD_LoadEntireFile(arena, js_path);
    
    //~ NOTE(rjf): Parse site info.
    SiteInfo site_info = {0};
    {
        printf("Parsing site metadata at \"%.*s\"...\n", MD_S8VArg(site_info_path));
        MD_Node *site_info_file = MD_ParseWholeFile(arena, site_info_path).node;
        site_info = ParseSiteInfo(site_info_file);
    }
    
    //~ NOTE(rjf): Parse pages.
    MD_Node *root_list = MD_MakeList(arena);
    {
        printf("Searching for site pages at \"%.*s\"...\n", MD_S8VArg(page_dir_path));
        MD_FileInfo file_info = {0};
        for(MD_FileIter it = {0}; MD_FileIterIncrement(arena, &it, page_dir_path, &file_info);)
        {
            if(MD_S8Match(MD_PathSkipLastPeriod(file_info.filename), MD_S8Lit("md"), MD_StringMatchFlag_CaseInsensitive) &&
               !MD_S8Match(MD_PathSkipLastSlash(MD_PathChopLastPeriod(file_info.filename)),
                           MD_PathSkipLastSlash(MD_PathChopLastPeriod(site_info_path)),
                           MD_StringMatchFlag_CaseInsensitive |
                           MD_StringMatchFlag_SlashInsensitive))
            {
                printf("Processing site page at \"%.*s\"...\n", MD_S8VArg(file_info.filename));
                MD_String8 folder = MD_PathChopLastSlash(page_dir_path);
                MD_String8 path = MD_S8Fmt(arena, "%.*s/%.*s",
                                           MD_S8VArg(folder), MD_S8VArg(file_info.filename));
                MD_Node *node = MD_ParseWholeFile(arena, path).node;
                MD_PushNewReference(arena, root_list, node);
            }
        }
    }
    
    //~ NOTE(rjf): Generate index table.
    MD_Map index_table = {0};
    {
        for(MD_EachNode(ref, root_list->first_child))
        {
            MD_Node *root = MD_NodeFromReference(ref);
            for(MD_EachNode(node, root->first_child))
            {
                if(!MD_NodeIsNil(node->first_child) && MD_S8Match(node->string, MD_S8Lit("index"), MD_StringMatchFlag_CaseInsensitive))
                {
                    for(MD_EachNode(index_string, node->first_child))
                    {
                        MD_MapInsert(arena, &index_table, MD_MapKeyStr(index_string->string), root);
                    }
                    goto end_index_build;
                }
            }
            end_index_build:;
        }
    }
    
    //~ NOTE(rjf): Generate stylesheet.
    if(site_info.style)
    {
        FILE *file = fopen("style.css", "wb");
        if(file)
        {
            fprintf(file, "%.*s", MD_S8VArg(site_info.style->string));
            fclose(file);
        }
    }
    
    //~ NOTE(rjf): Generate JS.
    {
        FILE *file = fopen("site.js", "wb");
        if(file)
        {
            fprintf(file, "%.*s", MD_S8VArg(js_string));
            fclose(file);
        }
    }
    
    //~ NOTE(rjf): Generate files for all roots.
    for(MD_EachNode(ref, root_list->first_child))
    {
        MD_Node *root = MD_NodeFromReference(ref);
        PageInfo page_info = ParsePageInfo(root);
        
        MD_String8 name_without_extension = MD_PathSkipLastSlash(MD_PathChopLastPeriod(root->string));
        MD_String8 file_name = MD_S8Fmt(arena, "%.*s.html", MD_S8VArg(name_without_extension));
        FILE *file = fopen(file_name.str, "wb");
        if(file)
        {
            fprintf(file, "<!DOCTYPE html>\n");
            fprintf(file, "<html lang=\"en\">\n");
            
            MD_String8 site_title      = !MD_NodeIsNil(site_info.title) ? site_info.title->string : MD_S8Lit("");
            MD_String8 title           = !MD_NodeIsNil(page_info.title) ? page_info.title->string : MD_S8Lit("");
            MD_String8 url             = !MD_NodeIsNil(site_info.canonical_url) ? site_info.canonical_url->string : MD_S8Lit("");
            MD_String8 author          = !MD_NodeIsNil(site_info.author) ? site_info.author->string : MD_S8Lit("");
            MD_String8 twitter_handle  = !MD_NodeIsNil(site_info.twitter_handle) ? site_info.twitter_handle->string : MD_S8Lit("");
            
            // NOTE(rjf): Generate heading.
            {
                fprintf(file, "<head>\n");
                fprintf(file, "<meta charset=\"utf-8\">\n");
                fprintf(file, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><meta name=\"author\" content=\"%.*s\">\n", MD_S8VArg(author));
                fprintf(file, "<meta property=\"og:title\" content=\"%.*s\">\n", MD_S8VArg(title));
                fprintf(file, "<meta name=\"twitter:title\" content=\"%.*s\">\n", MD_S8VArg(title));
                fprintf(file, "<link rel=\"canonical\" href=\"%.*s\">\n", MD_S8VArg(url));
                fprintf(file, "<meta property=\"og:type\" content=\"website\">\n");
                fprintf(file, "<meta property=\"og:url\" content=\"%.*s\">\n", MD_S8VArg(url));
                fprintf(file, "<meta property=\"og:site_name\" content=\"%.*s\">\n", MD_S8VArg(site_title));
                fprintf(file, "<meta name=\"twitter:card\" content=\"summary\">\n");
                fprintf(file, "<meta name=\"twitter:site\" content=\"%.*s\">\n", MD_S8VArg(twitter_handle));
                fprintf(file, "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">\n");
                fprintf(file, "<script src=\"site.js\"></script>\n");
                if(title.size > 0)
                {
                    if(site_title.size > 0)
                    {
                        fprintf(file, "<title>%.*s | %.*s</title>\n", MD_S8VArg(title), MD_S8VArg(site_title));
                    }
                    else
                    {
                        fprintf(file, "<title>%.*s</title>\n", MD_S8VArg(title));
                    }
                }
                else if(site_title.size > 0)
                {
                    fprintf(file, "<title>%.*s</title>\n", MD_S8VArg(site_title));
                }
                fprintf(file, "</head>\n");
            }
            
            // NOTE(rjf): Generate body.
            {
                fprintf(file, "<body>\n");
                
                // NOTE(rjf): Generate header.
                if(site_info.header)
                {
                    fprintf(file, "%.*s", MD_S8VArg(site_info.header->string));
                }
                
                fprintf(file, "<div class=\"page_content\">\n");
                
                // NOTE(rjf): Parent page back button.
                if(page_info.parent)
                {
                    fprintf(file, "<div class=\"standalone_link_container\"><a class=\"link\" href=\"%.*s.html\">← Back</a></div>", MD_S8VArg(page_info.parent->string));
                }
                
                // NOTE(rjf): Banner.
                if(page_info.header_image)
                {
                    fprintf(file, "<div class=\"page_banner\" style=\"background-image: url('%.*s');\"></div>",
                            MD_S8VArg(page_info.header_image->string));
                }
                
                // NOTE(rjf): Title.
                if(title.size > 0)
                {
                    fprintf(file, "<h1 class=\"title\">%.*s</h1>", MD_S8VArg(title));
                }
                
                // NOTE(rjf): Main description/subtitle.
                if(page_info.desc)
                {
                    fprintf(file, "<h2 class=\"subtitle\">%.*s</h2>", MD_S8VArg(page_info.desc->string));
                }
                
                // NOTE(rjf): Date.
                if(page_info.date)
                {
                    MD_String8 date_string = MakeDateString(page_info.date);
                    if(date_string.size > 0)
                    {
                        fprintf(file, "<h3 class=\"date\">%.*s</h3>", MD_S8VArg(date_string));
                    }
                }
                
                // NOTE(rjf): The rest of the page content should be generated from the page nodes.
                for(MD_EachNode(node, root->first_child))
                {
                    GeneratePageContent(&index_table, &site_info, &page_info, file, node);
                }
                
                fprintf(file, "</div>\n");
                
                // NOTE(rjf): Generate footer.
                if(site_info.footer)
                {
                    fprintf(file, "%.*s", MD_S8VArg(site_info.footer->string));
                }
                
                fprintf(file, "</body>\n");
            }
            
            fprintf(file, "</html>\n");
            fclose(file);
        }
    }
    
    end:;
    return 0;
}

static PageInfo
ParsePageInfo(MD_Node *page)
{
    PageInfo info = {0};
    for(MD_EachNode(node, page->first_child))
    {
        if(!MD_NodeIsNil(node->first_child))
        {
            if(MD_S8Match(node->string, MD_S8Lit("title"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.title = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("desc"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.desc = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("date"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.date = node;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("parent"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.parent = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("header_image"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.header_image = node->first_child;
            }
        }
    }
    return info;
}

static SiteInfo
ParseSiteInfo(MD_Node *site)
{
    SiteInfo info = {0};
    for(MD_EachNode(node, site->first_child))
    {
        if(!MD_NodeIsNil(node->first_child))
        {
            if(MD_S8Match(node->string, MD_S8Lit("title"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.title = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("desc"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.desc = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("canonical_url"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.canonical_url = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("author"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.author = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("twitter_handle"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.twitter_handle = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("link_dictionary"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.link_dictionary = node;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("header"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.header = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("footer"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.footer = node->first_child;
            }
            else if(MD_S8Match(node->string, MD_S8Lit("style"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.style = node->first_child;
            }
        }
    }
    return info;
}

static MD_String8
MakeDateString(MD_Node *date)
{
    MD_String8 result = {0};
    if(date)
    {
        MD_Node *year = 0;
        MD_Node *month = 0;
        MD_Node *day = 0;
        
        for(MD_EachNode(child, date->first_child))
        {
            if(child->flags & MD_NodeFlag_Numeric)
            {
                if      (year  == 0) year  = child;
                else if (month == 0) month = child;
                else if (day   == 0) day   = child;
                else
                {
                    break;
                }
            }
        }
        
        if(!MD_NodeIsNil(year) && !MD_NodeIsNil(month) && !MD_NodeIsNil(day))
        {
            char *month_names[] =
            {
                "January", "February", "March", "April", "May", "June", "July", "August",
                "September", "October", "November", "December",
            };
            MD_u64 month_idx = MD_U64FromString(month->string, 10) - 1;
            if(month_idx >= 0 && month_idx < sizeof(month_names)/sizeof(month_names[0]))
            {
                result = MD_S8Fmt(arena, "%.*s %s %.*s",
                                  MD_S8VArg(day->string),
                                  month_names[month_idx],
                                  MD_S8VArg(year->string));
            }
        }
    }
    
    return result;
}

static void
GeneratePageContent(MD_Map *index_table, SiteInfo *site_info, PageInfo *page_info, FILE *file, MD_Node *node)
{
    
    //~ NOTE(rjf): Text blobs
    if(MD_NodeIsNil(node->first_child) && (node->flags & MD_NodeFlag_StringLiteral))
    {
        char *html_tag = "p";
        char *style = "paragraph";
        if(MD_NodeHasTag(node, MD_S8Lit("title"), 0))
        {
            html_tag = "h1";
            style = "title";
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("subtitle"), 0))
        {
            html_tag = "h2";
            style = "subtitle";
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("code"), 0))
        {
            html_tag = "pre";
            style = "code";
        }
        
        MD_String8 splits[] =
        {
            MD_S8Lit("\n\n"),
        };
        MD_String8List strlist = MD_S8Split(arena, node->string, sizeof(splits)/sizeof(splits[0]),
                                            splits);
        
        for(MD_String8Node *strnode = strlist.first; strnode; strnode = strnode->next)
        {
            fprintf(file, "<%s class=\"%s\">", html_tag, style);
            for(MD_u64 i = 0; i < strnode->string.size; i += 1)
            {
                if(strnode->string.str[i] == '@')
                {
                    MD_String8 string_tail = MD_S8Skip(strnode->string, i);
                    MD_ParseResult parse =  MD_ParseOneNode(arena, string_tail, 0);
                    if(!MD_NodeIsNil(parse.node))
                    {
                        if(MD_NodeHasTag(node, MD_S8Lit("i"), 0))
                        {
                            fprintf(file, "<i>%.*s</i>", MD_S8VArg(parse.node->string));
                        }
                        else if(MD_NodeHasTag(node, MD_S8Lit("b"), 0))
                        {
                            fprintf(file, "<strong>%.*s</strong>", MD_S8VArg(parse.node->string));
                        }
                        else if(MD_NodeHasTag(node, MD_S8Lit("code"), 0))
                        {
                            fprintf(file, "<span class=\"inline_code\">%.*s</span>", MD_S8VArg(parse.node->string));
                        }
                        else if(MD_NodeHasTag(node, MD_S8Lit("link"), 0))
                        {
                            MD_Node *text = MD_ChildFromIndex(parse.node, 0);
                            MD_Node *link = MD_ChildFromIndex(parse.node, 1);
                            fprintf(file, "<a class=\"link\" href=\"%.*s\">%.*s</a>",
                                    MD_S8VArg(link->string),
                                    MD_S8VArg(text->string));
                        }
                    }
                    i += parse.string_advance - 1;
                }
                else
                {
                    MD_b32 dict_word = 0;
                    if(site_info->link_dictionary)
                    {
                        MD_Node *text = MD_NilNode();
                        MD_Node *link = MD_NilNode();
                        
                        for(MD_EachNode(dict_link, site_info->link_dictionary->first_child))
                        {
                            text = MD_ChildFromIndex(dict_link, 0);
                            link = MD_ChildFromIndex(dict_link, 1);
                            MD_String8 substring = MD_S8Substring(strnode->string, i, i+text->string.size);
                            if(MD_S8Match(substring, text->string, 0))
                            {
                                fprintf(file, "<a class=\"link\" href=\"%.*s\">%.*s</a>",
                                        MD_S8VArg(link->string),
                                        MD_S8VArg(text->string));
                                dict_word = 1;
                                i += text->string.size-1;
                                break;
                            }
                        }
                    }
                    
                    if(!dict_word)
                    {
                        fprintf(file, "%c", strnode->string.str[i]);
                    }
                }
            }
            fprintf(file, "</%s>\n", html_tag);
        }
    }
    
    if(!MD_NodeIsNil(node->first_child))
    {
        if(MD_NodeHasTag(node, MD_S8Lit("list"), 0))
        {
            fprintf(file, "<ul class=\"list\">\n");
            for(MD_EachNode(child, node->first_child))
            {
                if(MD_NodeIsNil(child->first_child))
                {
                    fprintf(file, "<li class=\"list_item\">\n");
                }
                GeneratePageContent(index_table, site_info, page_info, file, child);
                if(MD_NodeIsNil(child->first_child))
                {
                    fprintf(file, "</li>\n");
                }
            }
            fprintf(file, "</ul>\n");
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("img"), 0))
        {
            MD_Node *src = MD_ChildFromIndex(node, 0);
            MD_Node *alt = MD_ChildFromIndex(node, 1);
            fprintf(file, "<div class=\"img_container\"><img class=\"img\" src=\"%.*s\"></img></div>\n", MD_S8VArg(src->string));
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("youtube"), 0))
        {
            MD_Node *id = MD_ChildFromIndex(node, 0);
            fprintf(file, "<iframe width=\"560\" height=\"315\" src=\"https://www.youtube.com/embed/%.*s\" frameborder=\"0\" allow=\"accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture\" allowfullscreen></iframe>\n",
                    MD_S8VArg(id->string));
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("lister"), 0))
        {
            static int lister_idx = 0;
            fprintf(file, "<input autofocus id=\"lister_search_%i\" class=\"lister_search\" oninput=\"SearchInput(event, %i)\" onkeydown=\"SearchKeyDown(event, %i)\" placeholder=\"Filter...\"></input>", lister_idx, lister_idx, lister_idx);
            fprintf(file, "<ul id=\"lister_%i\" class=\"lister\">\n", lister_idx);
            lister_idx += 1;
            
            MD_Node *index_string = 0;
            for(MD_u64 idx = 0; !MD_NodeIsNil(index_string = MD_ChildFromIndex(node, idx)); idx += 1)
            {
                for(MD_MapSlot *slot = MD_MapLookup(index_table, MD_MapKeyStr(index_string->string));
                    slot; slot = slot->next)
                {
                    if(slot->val)
                    {
                        MD_Node *node = slot->val;
                        MD_Node *root = MD_RootFromNode(node);
                        PageInfo info = ParsePageInfo(node);
                        
                        MD_String8 filename = root->string;
                        MD_String8 filename_no_ext = MD_PathChopLastPeriod(MD_PathSkipLastSlash(filename));
                        MD_String8 link = MD_S8Fmt(arena, "%.*s.html", MD_S8VArg(filename_no_ext));
                        MD_String8 name = info.title->string;
                        MD_String8 date = MakeDateString(info.date);
                        
                        fprintf(file, "<a class=\"lister_item_link\" href=\"%.*s\">\n", MD_S8VArg(link));
                        fprintf(file, "<li class=\"lister_item\">\n");
                        
                        if(info.header_image)
                        {
                            fprintf(file, "<div class=\"lister_item_img\" style=\"background-image:url('%.*s');\">",
                                    MD_S8VArg(info.header_image->string));
                        }
                        
                        fprintf(file, "<div class=\"lister_item_text\">\n");
                        fprintf(file, "<div class=\"lister_item_title\">\n");
                        fprintf(file, "%.*s\n", MD_S8VArg(name));
                        fprintf(file, "</div>\n");
                        
                        if(date.size > 0)
                        {
                            fprintf(file, "<div class=\"lister_item_date\">\n");
                            fprintf(file, "%.*s\n", MD_S8VArg(date));
                            fprintf(file, "</div>\n");
                        }
                        
                        if(info.desc)
                        {
                            fprintf(file, "<div class=\"lister_item_desc\">\n");
                            fprintf(file, "%.*s\n", MD_S8VArg(info.desc->string));
                            fprintf(file, "</div>\n");
                        }
                        if(info.header_image)
                        {
                            fprintf(file, "</div>\n");
                        }
                        fprintf(file, "</div>\n");
                        fprintf(file, "</a>\n");
                        
                        fprintf(file, "</li>\n");
                    }
                }
            }
            fprintf(file, "</ul>\n");
        }
    }
}
