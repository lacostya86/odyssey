
/*
 * machinarium.
 *
 * Cooperative multitasking engine.
*/

#include <machinarium.h>
#include <machinarium_test.h>

#include <string.h>
#include <arpa/inet.h>

static void
server(void *arg)
{
	machine_io_t *server = machine_io_create();
	test(server != NULL);

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");
	sa.sin_port = htons(7778);
	int rc;
	rc = machine_bind(server, (struct sockaddr*)&sa);
	test(rc == 0);

	machine_io_t *client;
	rc = machine_accept(server, &client, 16, 1, UINT32_MAX);
	test(rc == 0);

	char msg[] = "hello world";
	rc = machine_write(client, msg, sizeof(msg), UINT32_MAX);
	test(rc == 0);

	rc = machine_close(client);
	test(rc == 0);
	machine_io_free(client);

	rc = machine_close(server);
	test(rc == 0);
	machine_io_free(server);
}

static void
client(void *arg)
{
	machine_io_t *client = machine_io_create();
	test(client != NULL);

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");
	sa.sin_port = htons(7778);
	int rc;
	rc = machine_connect(client, (struct sockaddr*)&sa, UINT32_MAX);
	test(rc == 0);

	char buf[16];
	rc = machine_read(client, buf, 12, UINT32_MAX);
	test(rc == 0);
	test(memcmp(buf, "hello world", 12) == 0);

	rc = machine_read(client, buf, 1, UINT32_MAX);
	/* eof */
	test(rc == -1);

	rc = machine_close(client);
	test(rc == 0);
	machine_io_free(client);
}

static void
test_cs(void *arg)
{
	int rc;
	rc = machine_coroutine_create(server, NULL);
	test(rc != -1);

	rc = machine_coroutine_create(client, NULL);
	test(rc != -1);
}

void
test_client_server0(void)
{
	machinarium_init();

	int id;
	id = machine_create("test", test_cs, NULL);
	test(id != -1);

	int rc;
	rc = machine_wait(id);
	test(rc != -1);

	machinarium_free();
}
