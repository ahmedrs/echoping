/*
 * DNS plugin. 
 * $Id$
 */

#define IN_PLUGIN
#include "../../echoping.h"

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

struct addrinfo name_server;
poptContext dns_poptcon;
char *request;
int type;
short use_tcp = FALSE;
short no_recurse = FALSE;

/* nsError stolen from Liu & Albitz check_soa (in their book "DNS and BIND") */

/****************************************************************
 * nsError -- Print an error message from h_errno for a failure *
 *     looking up NS records.  res_query() converts the DNS     *
 *     packet return code to a smaller list of errors and       *
 *     places the error value in h_errno.  There is a routine   *
 *     called herror() for printing out strings from h_errno    *
 *     like perror() does for errno.  Unfortunately, the        *
 *     herror() messages assume you are looking up address      *
 *     records for hosts.  In this program, we are looking up   *
 *     NS records for domains, so we need our own list of error *
 *     strings.                                                 *
 ****************************************************************/
void
nsError (error, domain)
     int error;
     char *domain;
{
  switch (error)
    {
    case HOST_NOT_FOUND:
      (void) fprintf (stderr, "Unknown domain: %s\n", domain);
      break;
    case NO_DATA:
      (void) fprintf (stderr, "No records for %s\n", domain);
      break;
    case TRY_AGAIN:
      (void) fprintf (stderr, "No response for query\n");
      break;
    default:
      (void) fprintf (stderr, "Unexpected error\n");
      break;
    }
}

void
dns_usage (const char *msg)
{
  if (msg)
    {
      printf ("Error: %s\n", msg);
    }
  poptPrintUsage (dns_poptcon, stdout, 0);
  exit (1);
}

char *
init (const int argc, const char **argv)
{
  int value;
  char *msg = malloc (256);
  char *type_name = NULL;
  /* popt variables */
  struct poptOption options[] = {
    {"request", 'r', POPT_ARG_STRING, &request, 0,
     "Request (a domain name) to send to the name server",
     "request"},
    {"type", 't', POPT_ARG_STRING, &type_name, 0,
     "Type of resources queried (A, MX, SOA, etc)",
     "type"},
    {"tcp", NULL, POPT_ARG_NONE, &use_tcp, 0,
     "Use TCP for the request (virtual circuit)",
     "tcp"},
    {"no-recurse", NULL, POPT_ARG_NONE, &no_recurse, 0,
     "Do not ask recursion",
     "no-recurse"},
    POPT_AUTOHELP POPT_TABLEEND
  };
  dns_poptcon = poptGetContext (NULL, argc,
				argv, options, POPT_CONTEXT_KEEP_FIRST);
  while ((value = poptGetNextOpt (dns_poptcon)) > 0)
    {
      if (value < -1)
	{
	  sprintf (msg, "%s: %s",
		   poptBadOption (dns_poptcon, POPT_BADOPTION_NOALIAS),
		   poptStrerror (value));
	  dns_usage (msg);
	}
    }
  if (request == NULL)
    dns_usage ("Mandatory request missing");
  if (type_name == NULL)
    type = T_A;
  else
    {				/* TODO: a better algorithm. Use dns_rdatatype_fromtext in BIND ? */
      if (!strcmp (type_name, "A"))
	type = T_A;
      else if (!strcmp (type_name, "AAAA"))
	type = T_AAAA;
      else if (!strcmp (type_name, "NS"))
	type = T_NS;
      else if (!strcmp (type_name, "SOA"))
	type = T_SOA;
      else if (!strcmp (type_name, "MX"))
	type = T_MX;
      else if (!strcmp (type_name, "SRV"))
	type = T_SRV;
      else if (!strcmp (type_name, "TXT"))
	type = T_TXT;
      else
	dns_usage ("Unknown type");
    }
  return "domain";
}

void
start (struct addrinfo *res)
{
  struct sockaddr name_server_sockaddr;
  struct sockaddr_in name_server_sockaddr_in;
  name_server = *res;
  name_server_sockaddr = *name_server.ai_addr;
  /* Converts a generic sockaddr to an IPv4 sockaddr_in */
  (void) memcpy ((void *) &name_server_sockaddr_in, &name_server_sockaddr,
		 sizeof (struct sockaddr));
  if (res_init () < 0)
    err_sys ("res_init");
  _res.nsaddr_list[0] = name_server_sockaddr_in;	/* TODO: and IPv6? Detect _resext with autoconf (*BSD) and use it */
  _res.nscount = 1;
  _res.options &= ~(RES_DNSRCH | RES_DEFNAMES | RES_NOALIASES);
  if (use_tcp)
    {
      _res.options &= RES_USEVC;
    }
  if (no_recurse)
    {
      _res.options &= ~RES_RECURSE;
    }
}

int
execute ()
{
  union
  {
    HEADER hdr;			/* defined in resolv.h */
    u_char buf[PACKETSZ];	/* defined in arpa/nameser.h */
  } response;			/* response buffers */
  int response_length;		/* buffer length */
  if ((response_length = res_query (request,	/* the domain we care about   */
				    C_IN,	/* Internet class records     */
				    type, (u_char *) & response,	/*response buffer */
				    sizeof (response)))	/*buffer size    */
      < 0)
    {				/*If negative    */
      nsError (h_errno, request);	/* report the error           */
      if (h_errno == TRY_AGAIN)
	return -1;		/* More luck next time? */
      else
	return -2;		/* Give in */
    }
  /* TODO: better analysis of the replies. For instance, replies can be in the authority section (delegation info) */
  return 0;
}

void
terminate ()
{
}
