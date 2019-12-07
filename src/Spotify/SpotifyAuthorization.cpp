/**
 * @file    SpotifyAPI.h
 * @author  Stefan Jahn <stefan.jahn332@gmail.com>
 * @brief   Class SpotifyAuthorization implementation
 */

#include "SpotifyAuthorization.h"

#include <chrono>
#include <memory>

#include "Types/Result.h"
#include "Utils/ConfigHandler.h"
#include "Utils/LoggingHandler.h"
#include "httpserver.hpp"

using namespace SpotifyApi;
using namespace httpserver;

SpotifyAuthorization::~SpotifyAuthorization() {
  stopServer();
}

TResultOpt SpotifyAuthorization::startServer(void) {
  VLOG(100) << "SpotifyAuthorization:: Start Server" << std::endl;

  // if(setupConfigParams() holds error return error
  // setup urlencode in building sendstring..
  auto readConfigRet = setupConfigParams();
  if (readConfigRet.has_value()) {
    return readConfigRet;
  }
  shutdownServer = false;
  mServerThread = std::make_unique<std::thread>(
      &SpotifyAuthorization::startServerThread, this);

  return std::nullopt;
}

void SpotifyAuthorization::startServerThread() {
  webserver ws = create_webserver(mPort);
  ws.register_resource("/", this, true);

  ws.start(false);
  while (!shutdownServer) {
    usleep(100000);
  };
}
void SpotifyAuthorization::stopServer() {
  if (!shutdownServer) {
    shutdownServer = true;
    mServerThread->join();
  }
}

std::string const &SpotifyAuthorization::getRefreshToken(void) {
  return mToken.getRefreshToken();
}
std::string const &SpotifyAuthorization::getAccessToken(void) {
  return mToken.getAccessToken();
}
TResultOpt SpotifyAuthorization::refreshAccessToken(void) {
  // check if refresh token is available
  if (mToken.getRefreshToken().empty()) {
    return Error(ErrorCode::InvalidValue, "No refresh token available");
  }

  // to be sure only one thread does the refreshment
  std::unique_lock mLock(mMutex);
  // now check if token is really expired, or other thread refreshed it
  int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  if (now < getExpiresAt()) {
    // token isnt expired yet
    return std::nullopt;
  }

  SpotifyAPI api;
  auto ret = api.refreshAccessToken(
      mToken.getRefreshToken(), mClientID, mClientSecret);

  if (auto error = std::get_if<Error>(&ret)) {
    LOG(ERROR) << "[SpotifyAuthorization] in refreshAccessToken: "
               << error->getErrorMessage() << std::endl;
    return *error;
  }

  auto token = std::get<Token>(ret);
  // set refresh token, because in refresh access token no new refresh token
  // gets returned
  token.setRefreshToken(mToken.getRefreshToken());
  mTokenReceiveTime = std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
  mToken = token;
  return std::nullopt;
}

__int64_t SpotifyAuthorization::getExpiresAt(void) {
  return mTokenReceiveTime + mToken.getExpiresIn() -
         10;  // reduce by 10 to be sure (networktime delays,...)
}

void SpotifyAuthorization::setScopes(std::string const &scopes) {
  mScopes = scopes;
}

std::string SpotifyAuthorization::getScopes(void) {
  return mScopes;
}

const std::shared_ptr<httpserver::http_response> SpotifyAuthorization::render(
    httpserver::http_request const &request) {
  // dispatch path
  std::cout << request.get_path() << std::endl;
  if (request.get_path().find("/spotifyLogin") != std::string::npos) {
    return loginHandler(request);
  } else if (request.get_path().find("/spotifyCallback") != std::string::npos) {
    return callbackHandler(request);
  } else {
    auto response = std::make_shared<http_response>(
        string_response("Invalid Endpoint", http::http_utils::http_not_found));
    return response;
  }
}

const std::shared_ptr<httpserver::http_response>
SpotifyAuthorization::loginHandler(httpserver::http_request const &request) {
  auto response = std::make_shared<http_response>(
      string_response("", http::http_utils::http_found));

  auto state = generateRandomString(16);

  // build redirection string with query
  std::string redirectString("https://accounts.spotify.com/authorize");
  redirectString.append("?client_id=").append(mClientID);
  redirectString.append("&response_type=").append("code");
  redirectString.append("&scope=").append(SpotifyAPI::stringUrlEncode(mScopes));
  redirectString.append("&redirect_uri=")
      .append(SpotifyAPI::stringUrlEncode(mRedirectUri));
  redirectString.append("&state=").append(state);

  std::cout << redirectString << std::endl;

  response->with_cookie("spotify_auth_state", state);
  response->with_header("Location", redirectString);
  return response;
}

const std::shared_ptr<httpserver::http_response>
SpotifyAuthorization::callbackHandler(httpserver::http_request const &request) {
  auto queryString = request.get_querystring();
  std::cout << queryString << std::endl;

  // check if the state is the same as the sent one
  std::cout << getFromQueryString(queryString, "code") << std::endl;
  std::cout << getFromQueryString(queryString, "state") << std::endl;

  if (getFromQueryString(queryString, "state") !=
      request.get_cookie("spotify_auth_state")) {
    std::cout << "SpotifyAuthorization: received state != sent state"
              << std::endl;
  }

  if (getFromQueryString(queryString, "error") != "") {
    std::cout << "SpotifyAuthorization: callback error, reason: "
              << getFromQueryString(queryString, "error") << std::endl;
  } else if (getFromQueryString(queryString, "code") != "") {
    // successfull
    SpotifyAPI spotify;
    auto ret = spotify.getAccessToken(AuthorizationCode,
                                      getFromQueryString(queryString, "code"),
                                      SpotifyAPI::stringUrlEncode(mRedirectUri),
                                      mClientID,
                                      mClientSecret);
    if (auto error = std::get_if<Error>(&ret)) {
      LOG(ERROR) << "[SpotifyAuthorization] in getAccessToken: "
                 << error->getErrorMessage() << std::endl;
      return std::make_shared<http_response>(
          string_response(std::string("Error: ") + error->getErrorMessage(),
                          http::http_utils::http_bad_request));
      ;
    }
    auto token = std::get<Token>(ret);
    mToken = token;
    mTokenReceiveTime = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    VLOG(100) << "access token: " << mToken.getAccessToken() << std::endl;
    VLOG(100) << "refresh token: " << mToken.getRefreshToken() << std::endl;
    VLOG(100) << "token type token: " << mToken.getTokenType() << std::endl;
    VLOG(100) << "scope: " << mToken.getScope() << std::endl;
    VLOG(100) << "expires in: " << mToken.getExpiresIn() << std::endl;

  } else {
    // requested query not received
    LOG(ERROR) << "[SpotifyAuthorization]: callback error, reason: invalid "
                  "query string"
               << std::endl;
    return std::make_shared<http_response>(
        string_response("SpotifyAuthorization callback error",
                        http::http_utils::http_bad_request));
  }

  return std::make_shared<http_response>(
      string_response("OK", http::http_utils::http_ok));
}

TResultOpt SpotifyAuthorization::setupConfigParams() {
  auto configHandler = ConfigHandler::getInstance();

  // get port
  auto port = configHandler->getValueInt(cSectionKey, cPortKey);
  if (std::holds_alternative<Error>(port)) {
    LOG(ERROR) << "[SpotifyAuthorization] no config " << cPortKey
               << " available" << std::endl;
    return std::get<Error>(port);
  }

  // get redirect uri
  auto redirectUri =
      configHandler->getValueString(cSectionKey, cRedirectUriKey);
  if (std::holds_alternative<Error>(redirectUri)) {
    LOG(ERROR) << "[SpotifyAuthorization] no config " << cRedirectUriKey
               << " available" << std::endl;
    return std::get<Error>(redirectUri);
  }

  // get client id
  auto clientId = configHandler->getValueString(cSectionKey, cClientIDKey);
  if (std::holds_alternative<Error>(clientId)) {
    LOG(ERROR) << "[SpotifyAuthorization] no config " << cClientIDKey
               << " available" << std::endl;
    return std::get<Error>(clientId);
  }

  // get client secret
  auto clientSecret =
      configHandler->getValueString(cSectionKey, cClientSecretKey);
  if (std::holds_alternative<Error>(clientSecret)) {
    LOG(ERROR) << "[SpotifyAuthorization] no config " << cClientSecretKey
               << " available" << std::endl;
    return std::get<Error>(clientSecret);
  }

  // get scopes
  auto scopes = configHandler->getValueString(cSectionKey, cScopesKey);
  if (std::holds_alternative<Error>(scopes)) {
    LOG(ERROR) << "[SpotifyAuthorization] no config " << cScopesKey
               << " available" << std::endl;
    return std::get<Error>(scopes);
  }

  // set members
  mScopes = std::get<std::string>(scopes);
  mPort = std::get<int>(port);
  mRedirectUri = std::get<std::string>(redirectUri);
  mClientID = std::get<std::string>(clientId);
  mClientSecret = std::get<std::string>(clientSecret);

  return std::nullopt;
}

std::string SpotifyAuthorization::generateRandomString(size_t length) {
  std::string text = "";
  std::string possible =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (size_t i = 0; i < length; i++) {
    text += possible[rand() % possible.size()];
  }
  return text;
};

std::string SpotifyAuthorization::getFromQueryString(std::string const &query,
                                                     std::string const &key) {
  std::string value;
  auto pos = query.find(key);
  if (pos != std::string::npos) {
    pos = query.find("=", pos);
    if ((pos != std::string::npos) && (pos + 1) < query.length()) {
      value = query.substr(pos + 1, query.find("&", pos));
    }
  }
  return value;
}
