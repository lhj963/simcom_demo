#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>


/*about libcurl API,you can refer the weisite https://curl.haxx.se/libcurl/c/*/

size_t write_callback(char *ptr,size_t size,size_t nmemb,void *userdata)
{
    FILE *fp = (FILE *)userdata;
    size_t return_size = fwrite(ptr,size,nmemb,fp);
    return return_size;
}

int http_get(char *url, char *rename)
{
    CURL *curl;
    CURLcode res;
    FILE *fp = NULL;

    if (url == NULL || rename == NULL)
    {
        printf("Param is NULL\n");
        return -1;
    }
    
    if (strlen(url) == 0 || strlen(rename) == 0)
    {
        printf("Param is error");
        return -1;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);

    char *version = curl_version();
    printf("libcur Version = %s\n", version);
    curl = curl_easy_init();
    if (curl)
    {
        /*set url,this must set*/
        curl_easy_setopt(curl, CURLOPT_URL, url);
        /*set callback, if not set ,the result will output to stdout*/
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
        fp = fopen(rename, "w+");
        if (NULL == fp)
        {
            printf("failed to creat %s.\n", rename);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return -1;
        }
        /*fp point the fourth param of write_callback*/
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        /*the function is sync,will block until return*/
        res = curl_easy_perform(curl);

        if (CURLE_OK == res)
        {
            char *ct = NULL;      
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);	  
            if ((CURLE_OK == res) && ct)
            {
                printf("Content-Type: %s\n", ct);
            }
            double downLength = 0;
            res = curl_easy_getinfo(curl,CURLINFO_CONTENT_LENGTH_DOWNLOAD,&downLength);
            if ((CURLE_OK==res) && downLength)
            {
                printf("FileLength=%.0f\n", downLength);
                printf("FileLength is -1 means the size is not known\n");
            }
        }
        else
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
        }
        fclose(fp);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}


int https_get(char *url, char *rename, int ca_flag, char *cainfo)
{
    CURL *curl;
    CURLcode res;
    FILE *fp = NULL;

    if (url == NULL || rename == NULL)
    {
        printf("Param is NULL\n");
        return -1;
    }

    if (strlen(url) == 0 || strlen(rename) == 0)
    {
        printf("Param is error");
        return -1;
    }
    if (ca_flag == 1 && cainfo == NULL)
    {
        printf("Param is NULL\n");
        return -1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
        fp = fopen(rename, "w+");
        if (NULL == fp)
        {
            printf("failed to creat %s.\n", rename);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return -1;
        }
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

//#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
        if (ca_flag == 0)
        {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }
        else if (ca_flag == 1)
        {
        	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
            curl_easy_setopt(curl, CURLOPT_CAINFO , cainfo);
        }
 
 //#endif
 
 //#ifdef SKIP_HOSTNAME_VERIFICATION
     /*
      * If the site you're connecting to uses a different host name that what
      * they have mentioned in their server certificate's commonName (or
      * subjectAltName) fields, libcurl will refuse to connect. You can skip
      * this check, but this will make the connection less secure.
      */
     //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
 //#endif
 
     /* Perform the request, res will get the return code */
         res = curl_easy_perform(curl);
     /* Check for errors */
         if (CURLE_OK == res)
         {
             char *ct = NULL;
             res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
             if ((CURLE_OK == res) && ct)
             {
                printf("Content-Type: %s\n", ct);
             }
             double downLength = 0;
             res = curl_easy_getinfo(curl,CURLINFO_CONTENT_LENGTH_DOWNLOAD,&downLength);
             if ((CURLE_OK==res) && downLength)
             {
                 printf("FileLength=%.0f\n", downLength);
             }
         }
         else
         {
             fprintf(stderr, "curl_easy_perform() failed: %s\n",
             curl_easy_strerror(res));
         }
        fclose(fp);
        curl_easy_cleanup(curl);
     }
    curl_global_cleanup();

    return 0;
}

int https_get_binary(char *url, char *rename, char *cainfo, char *client_cert_file, char *client_key_file)
{
    CURL *curl;
    CURLcode res;
    FILE *fp = NULL;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
        fp = fopen(rename, "w+");
        if (NULL == fp)
        {
            printf("failed to creat %s.\n", rename);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return -1;
        }

        /*  Enable SSL/TLS authentication  */

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);

        /* Set the client certificate and private key */

        curl_easy_setopt(curl, CURLOPT_SSLCERT, client_cert_file);
        curl_easy_setopt(curl, CURLOPT_SSLKEY, client_key_file);

        /* Set CA certificate*/

        curl_easy_setopt(curl, CURLOPT_CAINFO, cainfo);

        res = curl_easy_perform(curl);
        if(res == CURLE_OK)
        {
            char *ct = NULL;
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
            if ((CURLE_OK == res) && ct)
            {
               printf("Content-Type: %s\n", ct);
            }
            double downLength = 0;
            res = curl_easy_getinfo(curl,CURLINFO_CONTENT_LENGTH_DOWNLOAD,&downLength);
            if ((CURLE_OK==res) && downLength)
            {
                printf("FileLength=%.0f\n", downLength);
            }
        }else
        {
           printf("[%s] failed err: %s\n", __FUNCTION__, curl_easy_strerror(res));
        }
        fclose(fp);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    
    return 0;
}


