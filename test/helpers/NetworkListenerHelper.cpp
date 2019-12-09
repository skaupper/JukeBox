#include "NetworkListenerHelper.h"

#include <gtest/gtest.h>

#include "Utils/Serializer.h"
#include "json/json.hpp"

using namespace std;
using json = nlohmann::json;

void testGenerateSession(RestAPIFixture *fixture,
                         TSessionID const &sid,
                         optional<TPassword> const &expPw,
                         optional<string> const &expNickname,
                         size_t count) {
  optional<TPassword> pw;
  optional<string> nickname;

  // prepare json bodies
  json requestBody = {};
  if (expPw.has_value()) {
    requestBody["password"] = expPw.value();
  }
  if (expNickname.has_value()) {
    requestBody["nickname"] = expNickname.value();
  }
  json expResponseBody = {{"session_id", sid}};

  // do request
  fixture->listener.setResponseGenerateSession(sid);
  auto resp = fixture->post("/generateSession", requestBody.dump()).value();

  // check response
  ASSERT_EQ(resp.code, 200);
  ASSERT_EQ(json::parse(resp.body), expResponseBody);
  ASSERT_EQ(fixture->listener.getCountGenerateSession(), count);

  ASSERT_TRUE(fixture->listener.hasParametersGenerateSession());
  fixture->listener.getLastParametersGenerateSession(pw, nickname);
  ASSERT_FALSE(fixture->listener.hasParametersGenerateSession());

  ASSERT_EQ(pw, expPw);
  ASSERT_EQ(nickname, expNickname);
}

void testQueryTracks(RestAPIFixture *fixture,
                     string const &expPattern,
                     int expMaxEntries,
                     size_t count) {
  string pattern;
  int maxEntries;

  auto expTracks = fixture->gen.generateTracks(expMaxEntries);
  json expResponseBody = {{"tracks", json{}}};
  for (auto &&track : expTracks) {
    expResponseBody["tracks"].push_back(Serializer::serialize(track));
  }

  map<string, string> parameters{{
      {"pattern", expPattern},                        //
      {"max_entries", std::to_string(expMaxEntries)}  //
  }};

  // do request
  fixture->listener.setResponseQueryTracks(expTracks);
  auto resp = fixture->get("/queryTracks", parameters).value();

  // check response
  ASSERT_EQ(resp.code, 200);
  ASSERT_EQ(json::parse(resp.body), expResponseBody);
  ASSERT_EQ(fixture->listener.getCountQueryTracks(), count);

  ASSERT_TRUE(fixture->listener.hasParametersQueryTracks());
  fixture->listener.getLastParametersQueryTracks(pattern, maxEntries);
  ASSERT_FALSE(fixture->listener.hasParametersQueryTracks());

  ASSERT_EQ(pattern, expPattern);
  ASSERT_EQ(maxEntries, expMaxEntries);
}

void testGetCurrentQueues(RestAPIFixture *fixture,
                          TSessionID const &expSid,
                          int normalNr,
                          int adminNr,
                          bool playbackTrack,
                          size_t count) {
  TSessionID sid;

  auto expQueueStatus =
      fixture->gen.generateQueueStatus(normalNr, adminNr, playbackTrack);
  json expResponseBody = {
      {"currently_playing", json{}},  //
      {"normal_queue", json{}},       //
      {"admin_queue", json{}}         //
  };

  if (expQueueStatus.currentTrack.has_value()) {
    expResponseBody["currently_playing"] =
        Serializer::serialize(expQueueStatus.currentTrack.value());
  }
  for (auto &&t : expQueueStatus.normalQueue.tracks) {
    expResponseBody["normal_queue"].push_back(Serializer::serialize(t));
  }
  for (auto &&t : expQueueStatus.adminQueue.tracks) {
    expResponseBody["admin_queue"].push_back(Serializer::serialize(t));
  }

  map<string, string> parameters{{{"session_id", expSid}}};

  // do request
  fixture->listener.setResponseGetCurrentQueues(expQueueStatus);
  auto resp = fixture->get("/getCurrentQueues", parameters).value();

  // check response
  ASSERT_EQ(resp.code, 200);
  ASSERT_EQ(json::parse(resp.body), expResponseBody);
  ASSERT_EQ(fixture->listener.getCountGetCurrentQueues(), count);

  ASSERT_TRUE(fixture->listener.hasParametersGetCurrentQueues());
  fixture->listener.getLastParametersGetCurrentQueues(sid);
  ASSERT_FALSE(fixture->listener.hasParametersGetCurrentQueues());

  ASSERT_EQ(sid, expSid);
}

void testAddTrackToQueue(RestAPIFixture *fixture,
                         TSessionID const &expSid,
                         TTrackID const &expTrkid,
                         QueueType expQueueType,
                         int count) {
  TSessionID sid;
  TTrackID trkid;
  QueueType queueType;

  json requestBody{
      {"session_id", expSid},  //
      {"track_id", expTrkid}   //
  };

  string type = "";
  if (expQueueType == QueueType::Admin) {
    type = "admin";
  } else if (expQueueType == QueueType::Normal) {
    type = "normal";
  }
  requestBody["queue_type"] = type;

  // do request
  auto resp = fixture->post("/addTrackToQueue", requestBody.dump()).value();

  // check response
  ASSERT_EQ(resp.code, 200);
  ASSERT_EQ(json::parse(resp.body), json{});
  ASSERT_EQ(fixture->listener.getCountAddTrackToQueue(), count);

  ASSERT_TRUE(fixture->listener.hasParametersAddTrackToQueue());
  fixture->listener.getLastParametersAddTrackToQueue(sid, trkid, queueType);
  ASSERT_FALSE(fixture->listener.hasParametersAddTrackToQueue());

  ASSERT_EQ(sid, expSid);
  ASSERT_EQ(trkid, expTrkid);
  ASSERT_EQ(queueType, expQueueType);
}

void testVoteTrack(RestAPIFixture *fixture,
                   TSessionID const &expSid,
                   TTrackID const &expTrkid,
                   TVote vote,
                   int count) {
  // TODO
}

void testControlPlayer(RestAPIFixture *fixture,
                       TSessionID const &expSid,
                       PlayerAction action,
                       int count) {
  // TODO
}

void testMoveTrack(RestAPIFixture *fixture,
                   TSessionID const &expSid,
                   TTrackID const &expTrkid,
                   QueueType expQueueType,
                   int count) {
  TSessionID sid;
  TTrackID trkid;
  QueueType queueType;

  json requestBody{
      {"session_id", expSid},  //
      {"track_id", expTrkid}   //
  };

  string type = "";
  if (expQueueType == QueueType::Admin) {
    type = "admin";
  } else if (expQueueType == QueueType::Normal) {
    type = "normal";
  }
  requestBody["queue_type"] = type;

  // do request
  auto resp = fixture->put("/moveTrack", requestBody.dump()).value();

  // check response
  ASSERT_EQ(resp.code, 200);
  ASSERT_EQ(json::parse(resp.body), json{});
  ASSERT_EQ(fixture->listener.getCountMoveTrack(), count);

  ASSERT_TRUE(fixture->listener.hasParametersMoveTrack());
  fixture->listener.getLastParametersMoveTrack(sid, trkid, queueType);
  ASSERT_FALSE(fixture->listener.hasParametersMoveTrack());

  ASSERT_EQ(sid, expSid);
  ASSERT_EQ(trkid, expTrkid);
  ASSERT_EQ(queueType, expQueueType);
}

void testRemoveTrack(RestAPIFixture *fixture,
                     TSessionID const &expSid,
                     TTrackID const &expTrkid,
                     int count) {
  // TODO
}
