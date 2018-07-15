#pragma once
#include "nosql_database.h"
#include "../src/nosql_object.h"

#include <sys/epoll.h>

void server_db_init(redis_server * server);

