#include "http_parser.h"

namespace tao {

namespace http {

void on_request_method(void *data, const char *at, size_t length) {

}

void on_request_uri(void *data, const char *at, size_t length) {

}

void on_request_fragment(void *data, const char *at, size_t length) {

}

void on_request_path(void *data, const char *at, size_t length) {

}

void on_request_query(void *data, const char *at, size_t length) {

}

void on_request_version(void *data, const char *at, size_t length) {

}

void on_request_header_done(void *data, const char *at, size_t length) {

}

void on_request_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {

}

HttpRequestParser::HttpRequestParser() 
    :m_error(0){
    m_data.reset(new tao::http::HttpRequest);
    http_parser_init(&m_parser);
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query;
    m_parser.http_version = on_request_version;
    m_parser.header_done = on_request_header_done;
    m_parser.http_field = on_request_http_field;
    m_parser.data = this;
}

size_t HttpRequestParser::execute(char *data, size_t len)
{
    return size_t();
}
int HttpRequestParser::isFinished()
{
    return 0;
}
int HttpRequestParser::hasError()
{
    return 0;
}
uint64_t HttpRequestParser::getContentLength()
{
    return 0;
}
uint64_t HttpRequestParser::GetHttpRequestBufferSize()
{
    return 0;
}
uint64_t HttpRequestParser::GetHttpRequestMaxBodySize()
{
    return 0;
}


void on_response_reason(void *data, const char *at, size_t length) {

}

void on_response_status(void *data, const char *at, size_t length) {

}

void on_response_chunk(void *data, const char *at, size_t length) {

}

void on_response_version(void *data, const char *at, size_t length) {

}

void on_response_header_done(void *data, const char *at, size_t length) {

}

void on_response_last_chunk(void *data, const char *at, size_t length) {

}

void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {

}

HttpResponseParser::HttpResponseParser()
    :m_error(0){
    m_data.reset(new tao::http::HttpResponse);
    httpclient_parser_init(&m_parser);
    m_parser.reason_phrase = on_response_reason;
    m_parser.status_code = on_response_status;
    m_parser.chunk_size = on_response_chunk;
    m_parser.http_version = on_response_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;
    m_parser.data = this;
}
size_t HttpResponseParser::execute(char *data, size_t len, bool chunck)
{
    return size_t();
}
int HttpResponseParser::isFinished()
{
    return 0;
}
int HttpResponseParser::hasError()
{
    return 0;
}
uint64_t HttpResponseParser::getContentLength()
{
    return 0;
}
uint64_t HttpResponseParser::GetHttpResponseBufferSize()
{
    return 0;
}
uint64_t HttpResponseParser::GetHttpResponseMaxBodySize()
{
    return 0;
}
}
}