
#include "esp_http_server.h"

esp_err_t start_web_server(void);

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
    class WebServer
    {
    private:
        httpd_handle_t server = NULL;
        esp_err_t httpd_start_err;

    public:
        WebServer(/* args */);
        ~WebServer();
        virtual httpd_config_t MakeConfig() { return HTTPD_DEFAULT_CONFIG(); }

        esp_err_t AddUriHandler()
        {
            return ESP_OK;
        }
    };

    class UriHandlerBase
    {
    private:
    public:
        UriHandlerBase(/* args */);
        ~UriHandlerBase();
    };

    WebServer::WebServer()
    {
        httpd_config_t config = MakeConfig();
        httpd_start_err = httpd_start(&server, &config);
    }

    WebServer::~WebServer()
    {
        if (httpd_start_err == ESP_OK)
            httpd_stop(server);
    }

} // namespace WebServer
