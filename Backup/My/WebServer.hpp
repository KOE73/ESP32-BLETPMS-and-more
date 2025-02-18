
#pragma once

#include <functional>

#include "esp_log.h"
#include "esp_http_server.h"

/*
For memory to future
#define HTTP_METHOD_MAP(XX)  WebDAV


Flag values for http_parser.flags field F_CHUNKED F_CONNECTION_KEEP_ALIVE F_CONNECTION_CLOSE F_CONNECTION_UPGRADE F_TRAILING F_UPGRADE F_SKIPBODY F_CONTENTLENGTH

# Основные функции HTTP-сервера
httpd_start                     		Starts the HTTP server.                            	Запускает HTTP-сервер.
httpd_stop                      		Stops the HTTP server.                             	Останавливает HTTP-сервер.
httpd_register_uri_handler      		Registers a URI handler.                           	Регистрирует обработчик URI.
httpd_unregister_uri_handler   		    Unregisters a URI handler.                         	Удаляет обработчик URI.
httpd_unregister_uri_handlers 		    Unregisters all URI handlers.                      	Удаляет все обработчики URI.

# Работа с запросами и ответами
httpd_resp_send                 		Sends HTTP response.                               	Отправляет HTTP-ответ.
httpd_resp_send_chunk           		Sends a chunk of an HTTP response.                 	Отправляет фрагмент HTTP-ответа.
httpd_resp_send_err             		Sends an HTTP error response.                      	Отправляет HTTP-ошибку.
httpd_resp_set_status           		Sets the HTTP status code for the response.        	Устанавливает HTTP-статус ответа.
httpd_resp_set_type             		Sets the content type for the response.            	Устанавливает тип содержимого (Content-Type).
httpd_resp_set_hdr              		Sets a header in the HTTP response.                	Устанавливает заголовок HTTP-ответа.
httpd_resp_sendstr              		Sends a string as an HTTP response.                	Отправляет строковый HTTP-ответ.
httpd_resp_sendstr_chunk        		Sends a chunk of a string as an HTTP response.     	Отправляет фрагмент строкового HTTP-ответа.
httpd_resp_send_404             		Sends a 404 HTTP error response (Not Found).       	Отправляет ошибку 404 (Not Found).
httpd_resp_send_500             		Sends a 500 HTTP error response (Internal Error).  	Отправляет ошибку 500 (Internal Server Error).

# Работа с входящими данными
httpd_req_recv                  		Reads data from an HTTP request.                   	Читает данные из HTTP-запроса.
httpd_req_get_url_query_len     		Gets the length of the query string.               	Получает длину строки запроса (query string).
httpd_req_get_url_query_str     		Gets the query string.                             	Получает строку запроса (query string).
httpd_query_key_value           		Extracts a key-value pair from the query string.   	Извлекает значение параметра из строки запроса.

# Управление веб-сокетами
httpd_ws_send_frame            		    Sends a WebSocket frame.                           	Отправляет WebSocket-фрейм.
httpd_ws_send_frame_async      		    Sends a WebSocket frame asynchronously.            	Отправляет WebSocket-фрейм асинхронно.
httpd_ws_get_frame_type        		    Gets the type of a WebSocket frame.                	Определяет тип WebSocket-фрейма.
httpd_ws_recv_frame            		    Receives a WebSocket frame.                        	Получает WebSocket-фрейм.

# Другие вспомогательные функции
httpd_sess_trigger_close        		Closes an HTTP session.                            	Закрывает HTTP-сессию.
httpd_sess_set_descriptors      		Sets the file descriptors for an HTTP session.     	Устанавливает файловые дескрипторы HTTP-сессии.
httpd_sess_get_count            		Returns the number of active HTTP sessions.        	Возвращает количество активных сессий.


"Оформи вывод с функциями из esp_http_server.h в виде списка с выравниванием по табуляции и разделением на логические группы."

*/

namespace WebServer
{
    class UriHandlerBase;

    class WebServer
    {
    private:
        httpd_handle_t _httpd_handle = NULL;
        esp_err_t _httpd_start_err;
        std::vector<std::reference_wrapper<UriHandlerBase>> _handlers;

        void AddHandler(UriHandlerBase &handler);

    public:
        friend class UriHandlerBase;

        WebServer(/* args */);
        WebServer(bool started);
        ~WebServer();
        virtual httpd_config_t MakeConfig() { return HTTPD_DEFAULT_CONFIG(); }

        httpd_handle_t getServerHandle() const { return _httpd_handle; }

        esp_err_t Start();
        esp_err_t Stop();

        // esp_err_t AddUriHandler(UriHandlerBase &handler)
        //{
        //     return ESP_OK;
        // }
    };

    class UriHandlerBase
    {
    private:
        WebServer &_webServer;
        std::function<esp_err_t(httpd_req_t *)> _wrapper;
        const char *_uri;
        httpd_method_t _method = HTTP_GET;
        httpd_uri_t _httpd_uri;
        const char *_text = "oK";
        ssize_t _text_len = 2;

        // static esp_err_t index_get_handler1(httpd_req_t *req)
        //{
        //     httpd_resp_send(req, index_html_start, index_html_length);
        //     return ESP_OK;
        // }

    public:
        UriHandlerBase(WebServer &server);
        UriHandlerBase(WebServer &server, const char *uri);
        UriHandlerBase(WebServer &server, const char *uri, const char *text);
        UriHandlerBase(WebServer &server, const char *uri, const char *text, ssize_t buf_len);
        ~UriHandlerBase();

        httpd_method_t getMethod() const { return _method; }
        const char *getUri() const { return _uri; }
virtual const char*getText() const { return _text; };

        void Register();

        virtual esp_err_t Handler(httpd_req_t *req);

        WebServer &getWebServer() const { return _webServer; }
    };

} // namespace WebServer
