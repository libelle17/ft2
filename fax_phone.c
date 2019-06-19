/**
 * The libroutermanager project
 * Copyright (c) 2012-2014 Jan-Michael Brummer
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * TODO:
 *  - get rid of active_capi_connection?
 *  - merge connection-XXXX signals
 */

#include <gio/gio.h>
#include <string.h>
#include <stdlib.h>

#include "appobject-emit.h"
#include "net_monitor.h"
#include "profile.h"
// #include "router.h"
#include "audio.h"
// #include "call.h"
// #include "gstring.h"

#include "faxophone.h"
#include "fax.h"
// #include "sff.h"
#include "phone.h"
// #include "ringtone.h"

#define RM_ERROR 

extern int obverb;

/*static */struct session *session = NULL;
static gconstpointer net_event;

struct capi_connection *active_capi_connection = NULL;
#undef false
#ifdef false
/**
 * \brief Dial number via fax
 * \param tiff tiff file name
 * \param trg_no target number
 * \param suppress suppress number flag
 * \return capi connection pointer
 */
struct capi_connection *fax_dial(gchar *tiff, const gchar *trg_no, gboolean suppress)
{
  if (obverb) printf("Beginn fax_dial: %s, %s, %d\n",tiff,trg_no,suppress);
	struct profile *profile = profile_get_active();
	gint modem = g_settings_get_int(profile->settings, "fax-bitrate");
	gboolean ecm = g_settings_get_boolean(profile->settings, "fax-ecm");
	gint controller = g_settings_get_int(profile->settings, "fax-controller") + 1;
	gint cip = g_settings_get_int(profile->settings, "fax-cip");
	const gchar *src_no = g_settings_get_string(profile->settings, "fax-number");
	const gchar *header = g_settings_get_string(profile->settings, "fax-header");
	const gchar *ident = g_settings_get_string(profile->settings, "fax-ident");
	struct capi_connection *connection = NULL;
	gchar *target;

	if (EMPTY_STRING(src_no)) {
		emit_message(0, (gchar*)"Source MSN not set, cannot dial");
		return NULL;
	}
	
	target = call_canonize_number(trg_no);

	if (cip == 1) {
		cip = FAX_CIP;
		g_debug("Using 'ISDN Fax' id");
	} else {
		cip = SPEECH_CIP;
		g_debug("Using 'Analog Fax' id");
	}

	if (g_settings_get_boolean(profile->settings, "fax-sff")) {
		connection = sff_send(tiff, modem, ecm, controller, src_no, target, ident, header, suppress);
	} else {
		connection = fax_send(tiff, modem, ecm, controller, cip, src_no, target, ident, header, suppress);
	}
	g_free(target);
  if (obverb) printf("Ende fax_dial: %s, %s, %d\n",tiff,trg_no,suppress);

	return connection;
}

/**
 * \brief Dial number via phone
 * \param trg_no target number
 * \param suppress suppress number flag
 * \return capi connection pointer
 */
struct capi_connection *phone_dial(const gchar *trg_no, gboolean suppress)
{
	struct profile *profile = profile_get_active();
	gint controller = g_settings_get_int(profile->settings, "phone-controller") + 1;
	const gchar *src_no = g_settings_get_string(profile->settings, "phone-number");
	struct capi_connection *connection = NULL;
	gchar *target;

	if (EMPTY_STRING(src_no)) {
		emit_message(0, (gchar*)"Source MSN not set, cannot dial");
		return NULL;
	}

	target = call_canonize_number(trg_no);

	connection = phone_call(controller, src_no, target, suppress);

	g_free(target);

	return connection;
}
#endif

/**
 * \brief Connection ring handler
 * \param connection capi connection structure
 */
gboolean connection_ring_idle(gpointer data)
{
	struct capi_connection *capi_connection = (struct capi_connection*)data;
	struct connection *connection;

	active_capi_connection = capi_connection;

	g_debug("connection_ring() src %s trg %s", capi_connection->source, capi_connection->target);
	connection = connection_find_by_number(capi_connection->source);
#if ACCEPT_INTERN
	if (!connection && !strncmp(capi_connection->source, "**", 2)) {
		connection = connection_add_call(981, CONNECTION_TYPE_INCOMING, capi_connection->source, capi_connection->target);
	}
#endif

	g_debug("connection_ring() connection %p", connection);
	if (connection) {
		g_debug("connection_ring() set capi_connection %p", capi_connection);
		connection->priv = capi_connection;

		emit_connection_notify(connection);
	}

	return G_SOURCE_REMOVE;
}

void connection_ring(struct capi_connection *capi_connection)
{
	g_idle_add(connection_ring_idle, capi_connection);
}


/**
 * \brief Connection code handler
 * \param connection capi connection structure
 * \param code dtmf code
 */
void connection_code(struct capi_connection *connection, gint code)
{
	g_debug("connection_code(): code 0x%x", code);
}

/**
 * \brief Connection status handlers - emits connection-status signal
 * \param connection capi connection structure
 * \param status status code
 */
void connection_status(struct capi_connection *connection, gint status)
{
	if (obverb) printf("Beginn connection_status\n");
	emit_connection_status(status, connection);
}

gboolean connection_established_idle(gpointer data)
{
	struct capi_connection *connection = (struct capi_connection*)data;

	emit_connection_established(connection);

	return G_SOURCE_REMOVE;
}

void connection_established(struct capi_connection *connection)
{
	g_idle_add(connection_established_idle, connection);
}

gboolean connection_terminated_idle(gpointer data)
{
	struct capi_connection *connection = (struct capi_connection*)data;

	emit_connection_terminated(connection);

	return G_SOURCE_REMOVE;
}

void connection_terminated(struct capi_connection *connection)
{
	g_idle_add(connection_terminated_idle, connection);
}

struct session_handlers session_handlers = {
	audio_open, /* audio_open */
	audio_read, /* audio_read */
	audio_write, /* audio_write */
	audio_close, /* audio_close */

	connection_established, /* connection_established */
	connection_terminated, /* connection_terminated */
	connection_ring, /* connection_ring */
	connection_code, /* connection_code */
	connection_status, /* connection_status */
};

/**
 * \brief Faxophone connect
 * \param user_data faxophone plugin pointer
 * \return error code
 */
#ifdef false
gboolean faxophone_connect(gpointer user_data)
{
	struct profile *profile = profile_get_active();
	gboolean retry = TRUE;
	gchar* host=router_get_host(profile);
	gint phonecontr=g_settings_get_int(profile->settings, "phone-controller");
	if (obverb) printf("Beginn faxophone_connect, host: %s, phone-controller: %i\n",host,phonecontr);

again:
	session = faxophone_init(&session_handlers, router_get_host(profile), g_settings_get_int(profile->settings, "phone-controller") + 1);
	if (!session && retry) {
		/* Maybe the port is closed, try to activate it and try again */
		router_dial_number(profile, PORT_ISDN1, "#96*3*");
		g_usleep(G_USEC_PER_SEC * 2);
		retry = FALSE;
		goto again;
	}

	if (obverb) printf("Ende faxophone_connect, host: %s, phone-controller: %i\n",host,phonecontr);
	return session != NULL;
}
#endif
gboolean faxophone_connect(gpointer user_data)
{
//	struct profile *profile = profile_get_active();
	gboolean retry = TRUE;
	gchar* _host=(gchar*)"fritz.box"; // router_get_host(profile);
//	gint phonecontr=g_settings_get_int(profile->settings, "phone-controller");
	gint controller=5;
	if (obverb) printf("Beginn faxophone_connect, host: %s, phone-controller: %i\n",_host,controller/*phonecontr*/);
again:
	session = faxophone_init(&session_handlers, _host/*router_get_host(profile)*/, controller/*g_settings_get_int(profile->settings, "phone-controller") */+ 1);
	if (!session && retry) {
		// Maybe the port is closed, try to activate it and try again 
	return session != NULL;

#ifdef false
		waehle(PORT_ISDN1, "#96*3*");//router_dial_number(profile, PORT_ISDN1, "#96*3*");
		g_usleep(G_USEC_PER_SEC * 2);
#endif
		retry = FALSE;
		goto again;
	}
	if (obverb) printf("Ende faxophone_connect, host: %s, phone-controller: %i\n",_host,controller/*phonecontr*/);
	return session != NULL;
}

/**
 * \brief Network disconnect callback
 * \param user_data faxophone plugin pointer
 * \return TRUE
 */
gboolean faxophone_disconnect(gpointer user_data)
{
	if (obverb) printf("Beginn faxophone_disconnect\n");
	faxophone_close(TRUE);
	return TRUE;
}

/**
 * \brief Init faxophone support
 */
void faxophone_setup(void)
{
	if (obverb) printf("Beginn faxophone_setup\n");
	net_event = net_add_event(faxophone_connect, faxophone_disconnect, NULL);
}
