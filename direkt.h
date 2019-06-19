#include <string>
#include <vector>
#define EMPTY_STRING(x) (!(x) || !strlen(x))
extern const std::string nix;
std::string xmlex(std::string data,std::string lim,std::string lim2=nix,size_t *p2p=0);
// Active router structure 
//extern struct router *active_router/*=NULL*/;
/** Global router plugin list */
//extern GSList *router_list/*=NULL*/;
/** Router login blocked shield */
//extern gboolean router_login_blocked/*=FALSE*/;
int obverb{0};

class fbcl
{
  private:
    std::string usr;
    std::string pwd;
    std::string host;
	// neu erfunden
	public:
	int controller;
	/*
  private:
    int result;
    std::string usr;
    std::string pwd;
    std::string sid,si0;
    const char* data;
    size_t read;
    GTimer *stim;
    std::string url;
    std::string host;
    SoupMessage *msg;
    SoupSession *soup_session;
    std::vector<std::string> numn;
  public:
    int eingel;
		// aus router_info
//  gchar *host;
//	gchar *user;
//	gchar *password;
	gchar *name;
	gchar *version;
	gchar *serial;
	gchar *session_id;
	gchar *lang;
	gchar *annex;

	// neu erfunden
	int controller;
	int tamstick;
	int port;
	gchar *faxpfad;
	string faxheader;
	string faxmsn;
	string faxident;
	string faxvolume;
	vector<string> numbers;
	vector<string> phoneports;
	string countrycode;
	string internationalcallprefix;
	string areacode;
	string nationalcallprefix;
	string namedect;
	string nameanalog;
	string nameisdn;
	// Extend 
	gint box_id;
	gint maj_ver_id;
	gint min_ver_id;
	GTimer *session_timer;
	// Ende router_info
	*/

  public:
    fbcl();
//    void fb_login_05_50();
		void waehle(string nr);
		gboolean faxophone_connect_hier();
};
