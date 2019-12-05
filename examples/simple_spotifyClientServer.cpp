/**
 * @file    simple_spotifyClientServer.cpp
 * @author  Stefan Jahn <stefan.jahn332@gmail.com>
 * @brief   Minimal example using the spotify api
 */

#include <iostream>

#include "SpotifyApi/SpotifyAPI.h"
#include "SpotifyApi/SpotifyAPITypes.h"
#include "SpotifyApi/SpotifyAuthorization.h"

using namespace std;

int main(void) {
   SpotifyApi::SpotifyAuthorization auth;
   auth.startServer();

  //std::string accessToken("BQBoxQyiKxFjmzf78m4uhZSuN5h3yqykqxCn1KJkJ_OmKVWv1wCjKfWjKru54iFhO5iNWO_"
  //                        "U8EnTgMx9FnGm_0C6ZN1HjTTWuwiEUoMGMbVUpM-iu361Zk9rIIk5cKj0VE0ay6mvGo75Et-"
  //                        "MduQpYhm8FTexTLKxR0npMy691_Yh4V0pOWc0IX2Ae85ZVfPZJQ");
  //SpotifyApi::SpotifyAPI api;
  //api.getAvailableDevices(accessToken);
  //api.getCurrentPlayback(accessToken);

  return 0;
}