#include <gtest/gtest.h>

#include <iostream>
#include <memory>

#include "../src/Types/Result.h"
#include "../src/Utils/ConfigHandler.h"
#include "../src/Datastore/RAMDataStore.h"

using namespace std;

TEST(DataStoreTest, OneisOne) {
    ASSERT_EQ(1,1);
}

TEST(DataStoreTest, GetQueue_Empty){
    RAMDataStore ds;

    // Queue with no element returns empty queues
    auto res = ds.getQueue(QueueType::Admin);
    ASSERT_EQ(checkAlternativeError(res), false);
    Queue q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 0);
    res = ds.getQueue(QueueType::Normal);
    ASSERT_EQ(checkAlternativeError(res), false);
    q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 0);
}

TEST(DataStoreTest, GetQueue_1Normal){
    RAMDataStore ds;
    BaseTrack tr;
    tr.trackId = "ahbsjkfbasdjhfb";
    tr.title = "anfjbsjdfb";
    tr.album = "sjfsdjfs";
    tr.artist = "uashdbfisud";
    tr.duration = 100;
    tr.iconUri = "aishdkabsjdb";
    tr.addedBy = "aisdliahsld";

    ds.addTrack(tr, QueueType::Normal);

    auto res = ds.getQueue(QueueType::Admin);
    ASSERT_EQ(checkAlternativeError(res), false);
    Queue q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 0);
    res = ds.getQueue(QueueType::Normal);
    ASSERT_EQ(checkAlternativeError(res), false);
    q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 1);
    ASSERT_EQ(q.tracks[0].trackId, tr.trackId);
    ASSERT_EQ(q.tracks[0].title, tr.title);
    ASSERT_EQ(q.tracks[0].album, tr.album);
    ASSERT_EQ(q.tracks[0].artist, tr.artist);
    ASSERT_EQ(q.tracks[0].duration, tr.duration);
    ASSERT_EQ(q.tracks[0].iconUri, tr.iconUri);
    ASSERT_EQ(q.tracks[0].addedBy, tr.addedBy);
}

TEST(DataStoreTest, GetQueue_1Admin){
    RAMDataStore ds;
    BaseTrack tr;
    tr.trackId = "ahbsjkfbasdjhfb";
    tr.title = "anfjbsjdfb";
    tr.album = "sjfsdjfs";
    tr.artist = "uashdbfisud";
    tr.duration = 100;
    tr.iconUri = "aishdkabsjdb";
    tr.addedBy = "aisdliahsld";

    ds.addTrack(tr, QueueType::Admin);

    auto res = ds.getQueue(QueueType::Normal);
    ASSERT_EQ(checkAlternativeError(res), false);
    Queue q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 0);
    res = ds.getQueue(QueueType::Admin);
    ASSERT_EQ(checkAlternativeError(res), false);
    q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 1);
    ASSERT_EQ(q.tracks[0].trackId, tr.trackId);
    ASSERT_EQ(q.tracks[0].title, tr.title);
    ASSERT_EQ(q.tracks[0].album, tr.album);
    ASSERT_EQ(q.tracks[0].artist, tr.artist);
    ASSERT_EQ(q.tracks[0].duration, tr.duration);
    ASSERT_EQ(q.tracks[0].iconUri, tr.iconUri);
    ASSERT_EQ(q.tracks[0].addedBy, tr.addedBy);
}

TEST(DataStoreTest, GetQueue_1Admin1Normal){
    RAMDataStore ds;
    BaseTrack tr;
    tr.trackId = "ahbsjkfbasdjhfb";
    tr.title = "anfjbsjdfb";
    tr.album = "sjfsdjfs";
    tr.artist = "uashdbfisud";
    tr.duration = 100;
    tr.iconUri = "aishdkabsjdb";
    tr.addedBy = "aisdliahsld";

    ds.addTrack(tr, QueueType::Admin);
    ds.addTrack(tr, QueueType::Normal);

    auto res = ds.getQueue(QueueType::Normal);
    ASSERT_EQ(checkAlternativeError(res), false);
    Queue q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 1);
    ASSERT_EQ(q.tracks[0].trackId, tr.trackId);
    ASSERT_EQ(q.tracks[0].title, tr.title);
    ASSERT_EQ(q.tracks[0].album, tr.album);
    ASSERT_EQ(q.tracks[0].artist, tr.artist);
    ASSERT_EQ(q.tracks[0].duration, tr.duration);
    ASSERT_EQ(q.tracks[0].iconUri, tr.iconUri);
    ASSERT_EQ(q.tracks[0].addedBy, tr.addedBy);

    res = ds.getQueue(QueueType::Admin);
    ASSERT_EQ(checkAlternativeError(res), false);
    q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 1);
    ASSERT_EQ(q.tracks[0].trackId, tr.trackId);
    ASSERT_EQ(q.tracks[0].title, tr.title);
    ASSERT_EQ(q.tracks[0].album, tr.album);
    ASSERT_EQ(q.tracks[0].artist, tr.artist);
    ASSERT_EQ(q.tracks[0].duration, tr.duration);
    ASSERT_EQ(q.tracks[0].iconUri, tr.iconUri);
    ASSERT_EQ(q.tracks[0].addedBy, tr.addedBy);
}

TEST(DataStoreTest, GetQueue_2Each){
    RAMDataStore ds;
    BaseTrack tr1;
    tr1.trackId = "ahbsjkfbasdjhfb";
    tr1.title = "anfjbsjdfb";
    tr1.album = "sjfsdjfs";
    tr1.artist = "uashdbfisud";
    tr1.duration = 100;
    tr1.iconUri = "aishdkabsjdb";
    tr1.addedBy = "aisdliahsld";
    BaseTrack tr2;
    tr2.trackId = "ahbsj234kfbasdjhfb";
    tr2.title = "anfjb234sjdfb";
    tr2.album = "sjfs5345djfs";
    tr2.artist = "uash645dbfisud";
    tr2.duration = 10230;
    tr2.iconUri = "aishd234kabsjdb";
    tr2.addedBy = "aisdli6434ahsld";

    ds.addTrack(tr1, QueueType::Normal);
    ds.addTrack(tr2, QueueType::Normal);

    ds.addTrack(tr1, QueueType::Admin);
    ds.addTrack(tr2, QueueType::Admin);

    auto res = ds.getQueue(QueueType::Admin);
    ASSERT_EQ(checkAlternativeError(res), false);
    Queue q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 2);
    ASSERT_EQ(q.tracks[0].trackId, tr1.trackId);
    ASSERT_EQ(q.tracks[0].title, tr1.title);
    ASSERT_EQ(q.tracks[0].album, tr1.album);
    ASSERT_EQ(q.tracks[0].artist, tr1.artist);
    ASSERT_EQ(q.tracks[0].duration, tr1.duration);
    ASSERT_EQ(q.tracks[0].iconUri, tr1.iconUri);
    ASSERT_EQ(q.tracks[0].addedBy, tr1.addedBy);
    ASSERT_EQ(q.tracks[1].trackId, tr2.trackId);
    ASSERT_EQ(q.tracks[1].title, tr2.title);
    ASSERT_EQ(q.tracks[1].album, tr2.album);
    ASSERT_EQ(q.tracks[1].artist, tr2.artist);
    ASSERT_EQ(q.tracks[1].duration, tr2.duration);
    ASSERT_EQ(q.tracks[1].iconUri, tr2.iconUri);
    ASSERT_EQ(q.tracks[1].addedBy, tr2.addedBy);

    res = ds.getQueue(QueueType::Normal);
    ASSERT_EQ(checkAlternativeError(res), false);
    q = get<Queue>(res);
    ASSERT_EQ(q.tracks.size(), 2);
    ASSERT_EQ(q.tracks[0].trackId, tr1.trackId);
    ASSERT_EQ(q.tracks[0].title, tr1.title);
    ASSERT_EQ(q.tracks[0].album, tr1.album);
    ASSERT_EQ(q.tracks[0].artist, tr1.artist);
    ASSERT_EQ(q.tracks[0].duration, tr1.duration);
    ASSERT_EQ(q.tracks[0].iconUri, tr1.iconUri);
    ASSERT_EQ(q.tracks[0].addedBy, tr1.addedBy);
    ASSERT_EQ(q.tracks[1].trackId, tr2.trackId);
    ASSERT_EQ(q.tracks[1].title, tr2.title);
    ASSERT_EQ(q.tracks[1].album, tr2.album);
    ASSERT_EQ(q.tracks[1].artist, tr2.artist);
    ASSERT_EQ(q.tracks[1].duration, tr2.duration);
    ASSERT_EQ(q.tracks[1].iconUri, tr2.iconUri);
    ASSERT_EQ(q.tracks[1].addedBy, tr2.addedBy);
}