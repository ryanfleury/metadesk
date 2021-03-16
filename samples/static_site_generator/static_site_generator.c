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
static void GeneratePageContent(MD_NodeTable *index_table, SiteInfo *site_info, PageInfo *page_info, FILE *file, MD_Node *node);

int main(int argument_count, char **arguments)
{
    
    //~ NOTE(rjf): Parse command line arguments.
    MD_String8 site_info_path = {0};
    MD_String8 page_dir_path = {0};
    MD_CommandLine cmdln = MD_CommandLine_Start(argument_count, arguments);
    if(!MD_CommandLine_FlagString(&cmdln, MD_S8Lit("--siteinfo"), &site_info_path) ||
       !MD_CommandLine_FlagString(&cmdln, MD_S8Lit("--pagedir"), &page_dir_path))
    {
        fprintf(stderr, "USAGE: %s --siteinfo <path to site info file> --pagedir <path to directory with pages> ...\n", arguments[0]);
        goto end;
    }
    
    //~ NOTE(rjf): Load JS.
    MD_String8 js_string = MD_LoadEntireFile(MD_PushStringF("%.*s/site.js", MD_StringExpand(page_dir_path)));
    
    //~ NOTE(rjf): Parse site info.
    SiteInfo site_info = {0};
    {
        printf("Parsing site metadata at \"%.*s\"...\n", MD_StringExpand(site_info_path));
        MD_Node *site_info_file = MD_ParseWholeFile(site_info_path).node;
        site_info = ParseSiteInfo(site_info_file);
    }
    
    //~ NOTE(rjf): Parse pages.
    MD_Node *first_root = MD_NilNode();
    MD_Node *last_root = MD_NilNode();
    {
        printf("Searching for site pages at \"%.*s\"...\n", MD_StringExpand(page_dir_path));
        MD_FileInfo file_info = {0};
        for(MD_FileIter it = {0}; MD_FileIterIncrement(&it, page_dir_path, &file_info);)
        {
            if(MD_StringMatch(MD_ExtensionFromPath(file_info.filename), MD_S8Lit("md"), MD_StringMatchFlag_CaseInsensitive) &&
               !MD_StringMatch(MD_SkipFolder(MD_ChopExtension(file_info.filename)),
                               MD_SkipFolder(MD_ChopExtension(site_info_path)),
                               MD_StringMatchFlag_CaseInsensitive |
                               MD_StringMatchFlag_SlashInsensitive))
            {
                printf("Processing site page at \"%.*s\"...\n", MD_StringExpand(file_info.filename));
                MD_String8 folder = MD_FolderFromPath(page_dir_path);
                MD_String8 path = MD_PushStringF("%.*s/%.*s",
                                                 MD_StringExpand(folder),
                                                 MD_StringExpand(file_info.filename));
                MD_PushSibling(&first_root, &last_root, MD_NilNode(), MD_ParseWholeFile(path).node);
            }
        }
    }
    
    //~ NOTE(rjf): Generate index table.
    MD_NodeTable index_table = {0};
    {
        for(MD_EachNode(root, first_root))
        {
            for(MD_EachNode(node, root->first_child))
            {
                if(!MD_NodeIsNil(node->first_child) && MD_StringMatch(node->string, MD_S8Lit("index"), MD_StringMatchFlag_CaseInsensitive))
                {
                    for(MD_EachNode(index_string, node->first_child))
                    {
                        MD_NodeTable_Insert(&index_table, MD_NodeTableCollisionRule_Chain, index_string->string, root);
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
        FILE *file = fopen("style.css", "w");
        if(file)
        {
            fprintf(file, "%.*s", MD_StringExpand(site_info.style->string));
            fclose(file);
        }
    }
    
    //~ NOTE(rjf): Generate JS.
    {
        FILE *file = fopen("site.js", "w");
        if(file)
        {
            fprintf(file, "%.*s", MD_StringExpand(js_string));
            fclose(file);
        }
    }
    
    //~ NOTE(rjf): Generate files for all roots.
    for(MD_EachNode(root, first_root))
    {
        PageInfo page_info = ParsePageInfo(root);
        
        MD_String8 name_without_extension = MD_SkipFolder(MD_ChopExtension(root->filename));
        FILE *file = fopen(MD_PushStringF("%.*s.html", MD_StringExpand(name_without_extension)).str, "w");
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
                fprintf(file, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><meta name=\"author\" content=\"%.*s\">\n", MD_StringExpand(author));
                fprintf(file, "<meta property=\"og:title\" content=\"%.*s\">\n", MD_StringExpand(title));
                fprintf(file, "<meta name=\"twitter:title\" content=\"%.*s\">\n", MD_StringExpand(title));
                fprintf(file, "<link rel=\"canonical\" href=\"%.*s\">\n", MD_StringExpand(url));
                fprintf(file, "<meta property=\"og:type\" content=\"website\">\n");
                fprintf(file, "<meta property=\"og:url\" content=\"%.*s\">\n", MD_StringExpand(url));
                fprintf(file, "<meta property=\"og:site_name\" content=\"%.*s\">\n", MD_StringExpand(site_title));
                fprintf(file, "<meta name=\"twitter:card\" content=\"summary\">\n");
                fprintf(file, "<meta name=\"twitter:site\" content=\"%.*s\">\n", MD_StringExpand(twitter_handle));
                fprintf(file, "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">\n");
                fprintf(file, "<script src=\"site.js\"></script>\n");
                if(title.size > 0)
                {
                    if(site_title.size > 0)
                    {
                        fprintf(file, "<title>%.*s | %.*s</title>\n", MD_StringExpand(title), MD_StringExpand(site_title));
                    }
                    else
                    {
                        fprintf(file, "<title>%.*s</title>\n", MD_StringExpand(title));
                    }
                }
                else if(site_title.size > 0)
                {
                    fprintf(file, "<title>%.*s</title>\n", MD_StringExpand(site_title));
                }
                fprintf(file, "</head>\n");
            }
            
            // NOTE(rjf): Generate body.
            {
                fprintf(file, "<body>\n");
                
                // NOTE(rjf): Generate header.
                if(site_info.header)
                {
                    fprintf(file, "%.*s", MD_StringExpand(site_info.header->string));
                }
                
                fprintf(file, "<div class=\"page_content\">\n");
                
                // NOTE(rjf): Parent page back button.
                if(page_info.parent)
                {
                    fprintf(file, "<div class=\"standalone_link_container\"><a class=\"link\" href=\"%.*s.html\">← Back</a></div>", MD_StringExpand(page_info.parent->string));
                }
                
                // NOTE(rjf): Banner.
                if(page_info.header_image)
                {
                    fprintf(file, "<div class=\"page_banner\" style=\"background-image: url('%.*s');\"></div>",
                            MD_StringExpand(page_info.header_image->string));
                }
                
                // NOTE(rjf): Title.
                if(title.size > 0)
                {
                    fprintf(file, "<h1 class=\"title\">%.*s</h1>", MD_StringExpand(title));
                }
                
                // NOTE(rjf): Main description/subtitle.
                if(page_info.desc)
                {
                    fprintf(file, "<h2 class=\"subtitle\">%.*s</h2>", MD_StringExpand(page_info.desc->string));
                }
                
                // NOTE(rjf): Date.
                if(page_info.date)
                {
                    MD_String8 date_string = MakeDateString(page_info.date);
                    if(date_string.size > 0)
                    {
                        fprintf(file, "<h3 class=\"date\">%.*s</h3>", MD_StringExpand(date_string));
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
                    fprintf(file, "%.*s", MD_StringExpand(site_info.footer->string));
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
            if(MD_StringMatch(node->string, MD_S8Lit("title"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.title = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("desc"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.desc = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("date"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.date = node;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("parent"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.parent = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("header_image"), MD_StringMatchFlag_CaseInsensitive))
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
            if(MD_StringMatch(node->string, MD_S8Lit("title"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.title = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("desc"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.desc = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("canonical_url"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.canonical_url = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("author"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.author = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("twitter_handle"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.twitter_handle = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("link_dictionary"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.link_dictionary = node;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("header"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.header = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("footer"), MD_StringMatchFlag_CaseInsensitive))
            {
                info.footer = node->first_child;
            }
            else if(MD_StringMatch(node->string, MD_S8Lit("style"), MD_StringMatchFlag_CaseInsensitive))
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
        
        if(year && month && day)
        {
            char *month_names[] =
            {
                "January", "February", "March", "April", "May", "June", "July", "August",
                "September", "October", "November", "December",
            };
            int month_idx = MD_I64FromString(month->string, 10)-1;
            if(month_idx >= 0 && month_idx < sizeof(month_names)/sizeof(month_names[0]))
            {
                result = MD_PushStringF("%.*s %s %.*s",
                                        MD_StringExpand(day->string),
                                        month_names[month_idx],
                                        MD_StringExpand(year->string));
            }
        }
    }
    
    return result;
}

static void
GeneratePageContent(MD_NodeTable *index_table, SiteInfo *site_info, PageInfo *page_info, FILE *file, MD_Node *node)
{
    
    //~ NOTE(rjf): Text blobs
    if(MD_NodeIsNil(node->first_child) &&
       (node->flags & MD_NodeFlag_StringLiteral ||
        node->flags & MD_NodeFlag_CharLiteral))
    {
        char *html_tag = "p";
        char *style = "paragraph";
        if(MD_NodeHasTag(node, MD_S8Lit("title")))
        {
            html_tag = "h1";
            style = "title";
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("subtitle")))
        {
            html_tag = "h2";
            style = "subtitle";
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("code")))
        {
            html_tag = "pre";
            style = "code";
        }
        
        MD_String8 splits[] =
        {
            MD_S8Lit("\n\n"),
        };
        MD_String8List strlist = MD_SplitString(node->string, sizeof(splits)/sizeof(splits[0]), splits);
        
        for(MD_String8Node *strnode = strlist.first; strnode; strnode = strnode->next)
        {
            fprintf(file, "<%s class=\"%s\">", html_tag, style);
            for(MD_u64 i = 0; i < strnode->string.size; i += 1)
            {
                if(strnode->string.str[i] == '@')
                {
                    MD_ParseResult parse =  MD_ParseOneNode(node->filename, MD_StringSubstring(strnode->string, i, strnode->string.size));
                    if(!MD_NodeIsNil(parse.node))
                    {
                        if(MD_NodeHasTag(node, MD_S8Lit("i")))
                        {
                            fprintf(file, "<i>%.*s</i>", MD_StringExpand(parse.node->string));
                        }
                        else if(MD_NodeHasTag(node, MD_S8Lit("b")))
                        {
                            fprintf(file, "<strong>%.*s</strong>", MD_StringExpand(parse.node->string));
                        }
                        else if(MD_NodeHasTag(node, MD_S8Lit("code")))
                        {
                            fprintf(file, "<span class=\"inline_code\">%.*s</span>", MD_StringExpand(parse.node->string));
                        }
                        else if(MD_NodeHasTag(node, MD_S8Lit("link")))
                        {
                            MD_Node *text = MD_ChildFromIndex(parse.node, 0);
                            MD_Node *link = MD_ChildFromIndex(parse.node, 1);
                            fprintf(file, "<a class=\"link\" href=\"%.*s\">%.*s</a>",
                                    MD_StringExpand(link->string),
                                    MD_StringExpand(text->string));
                        }
                    }
                    i += parse.bytes_parsed - 1;
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
                            MD_String8 substring = MD_StringSubstring(strnode->string, i, i+text->string.size);
                            if(MD_StringMatch(substring, text->string, 0))
                            {
                                fprintf(file, "<a class=\"link\" href=\"%.*s\">%.*s</a>",
                                        MD_StringExpand(link->string),
                                        MD_StringExpand(text->string));
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
        if(MD_NodeHasTag(node, MD_S8Lit("list")))
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
        else if(MD_NodeHasTag(node, MD_S8Lit("img")))
        {
            MD_Node *src = MD_ChildFromIndex(node, 0);
            MD_Node *alt = MD_ChildFromIndex(node, 1);
            fprintf(file, "<div class=\"img_container\"><img class=\"img\" src=\"%.*s\"></img></div>\n", MD_StringExpand(src->string));
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("youtube")))
        {
            MD_Node *id = MD_ChildFromIndex(node, 0);
            fprintf(file, "<iframe width=\"560\" height=\"315\" src=\"https://www.youtube.com/embed/%.*s\" frameborder=\"0\" allow=\"accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture\" allowfullscreen></iframe>\n",
                    MD_StringExpand(id->string));
        }
        else if(MD_NodeHasTag(node, MD_S8Lit("lister")))
        {
            static int lister_idx = 0;
            fprintf(file, "<input autofocus id=\"lister_search_%i\" class=\"lister_search\" oninput=\"SearchInput(event, %i)\" onkeydown=\"SearchKeyDown(event, %i)\" placeholder=\"Filter...\"></input>", lister_idx, lister_idx, lister_idx);
            fprintf(file, "<ul id=\"lister_%i\" class=\"lister\">\n", lister_idx);
            lister_idx += 1;
            
            MD_Node *index_string = 0;
            for(MD_u64 idx = 0; !MD_NodeIsNil(index_string = MD_ChildFromIndex(node, idx)); idx += 1)
            {
                for(MD_NodeTableSlot *slot = MD_NodeTable_Lookup(index_table, index_string->string);
                    slot; slot = slot->next)
                {
                    if(slot->value)
                    {
                        PageInfo info = ParsePageInfo((MD_Node *)slot->value);
                        
                        MD_String8 filename = ((MD_Node *)slot->value)->filename;
                        MD_String8 filename_no_ext = MD_ChopExtension(MD_SkipFolder(filename));
                        MD_String8 link = MD_PushStringF("%.*s.html", MD_StringExpand(filename_no_ext));
                        MD_String8 name = info.title->string;
                        MD_String8 date = MakeDateString(info.date);
                        
                        fprintf(file, "<a class=\"lister_item_link\" href=\"%.*s\">\n", MD_StringExpand(link));
                        fprintf(file, "<li class=\"lister_item\">\n");
                        
                        if(info.header_image)
                        {
                            fprintf(file, "<div class=\"lister_item_img\" style=\"background-image:url('%.*s');\">",
                                    MD_StringExpand(info.header_image->string));
                        }
                        
                        fprintf(file, "<div class=\"lister_item_text\">\n");
                        fprintf(file, "<div class=\"lister_item_title\">\n");
                        fprintf(file, "%.*s\n", MD_StringExpand(name));
                        fprintf(file, "</div>\n");
                        
                        if(date.size > 0)
                        {
                            fprintf(file, "<div class=\"lister_item_date\">\n");
                            fprintf(file, "%.*s\n", MD_StringExpand(date));
                            fprintf(file, "</div>\n");
                        }
                        
                        if(info.desc)
                        {
                            fprintf(file, "<div class=\"lister_item_desc\">\n");
                            fprintf(file, "%.*s\n", MD_StringExpand(info.desc->string));
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
