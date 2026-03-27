#ifndef CONFIG_H
#define CONFIG_H

#ifndef VERSION
    #define VERSION "0.99.5"
#endif

#ifndef PACKAGE
    #define PACKAGE "DuneCity"
#endif

#define VERSIONSTRING   PACKAGE VERSION

#ifndef DUNELEGACY_DATADIR
    #define DUNELEGACY_DATADIR "/usr/local/share/DuneCity"
#endif

#ifndef CONFIGFILENAME
    #define CONFIGFILENAME "Dune Legacy.ini"
#endif

#ifndef LOGFILENAME
    #define LOGFILENAME "Dune Legacy.log"
#endif

/* #undef HAS_GETHOSTBYADDR_R */
/* #undef HAS_GETHOSTBYNAME_R */
/* #undef HAS_POLL */
/* #undef HAS_FCNTL */
/* #undef HAS_INET_PTON */
/* #undef HAS_INET_NTOP */
/* #undef HAS_MSGHDR_FLAGS */
/* #undef HAS_SOCKLEN_T */
/* #undef ENET_BUFFER_MAXIMUM */

#endif // CONFIG_H 
