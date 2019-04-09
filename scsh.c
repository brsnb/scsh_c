#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _string
{
    char *str;
    size_t len;
} string;

/*
    Initializes a string type with the data in content, then null terminates it.
        - content equal to NULL or empty string ititializes to an empty string
*/
void string_init(string *s, char *content)
{
    if (content == NULL || content[0] == '\0')
    {
        s->len = 0L;
        s->str = malloc(1);
        if (s->str == NULL)
        {
            fprintf(stdout, "Error creating new string\n");
            exit(EXIT_FAILURE);
        }
        s->str[0] = '\0';
    }
    else
    {
        s->len = strlen(content);
        s->str = malloc(s->len+1);
        if (s->str == NULL)
        {
            fprintf(stdout, "Error creating new string\n");
            exit(EXIT_FAILURE);
        }
        memcpy(s->str, content, s->len);
        s->str[s->len+1] = '\0';
    }
}

size_t write_func(char *content, size_t size, size_t num_members, void *user_data)
{
    size_t real_size = size * num_members;
    string *data = (string *)user_data;

    char *ptr = realloc(data->str, data->len+real_size+1);

    if (ptr == NULL)
    {
        fprintf(stdout, "Error realloc() failed\n");
        exit(EXIT_FAILURE);
    }

    data->str = ptr;
    memcpy(&(data->str[data->len]), content, real_size);
    data->len += real_size;
    data->str[data->len] = '\0';

    return real_size;
}

string post_image(string *filename)
{
    CURL *curl = NULL;
    CURLcode ret_code;

    string response;
    string_init(&response, "");

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl)
    {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Authorization: Client-ID 8d7d119014fab88");
        headers = curl_slist_append(headers, "Expect: ");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
        curl_easy_setopt(curl, CURLOPT_URL,           "https://api.imgur.com/3/image");
        //curl_easy_setopt(curl, CURLOPT_URL,           "localhost:6666");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,     (void *) &response);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_mime *multipart = curl_mime_init(curl);
        curl_mimepart *part = curl_mime_addpart(multipart);
        curl_mime_name(part, "image");
        curl_mime_filedata(part, filename->str);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);
        
        ret_code = curl_easy_perform(curl);
        if (ret_code != CURLE_OK)
        {
            fprintf(stdout, "POST failed %s\n", curl_easy_strerror(ret_code));
        }

        curl_mime_free(multipart);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return response;
}


int main(void)
{
    string filename;
    string_init(&filename, "/home/nb/misc/food.png");
    string response = post_image(&filename);
    printf("%s\n", response.str);

    return 0;
}