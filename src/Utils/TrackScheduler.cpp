/*****************************************************************************/
/**
 * @file    TrackScheduler.cpp
 * @author  Michael Wurm <wurm.michael95@gmail.com>
 * @brief   Class TrackScheduler implementation
 */
/*****************************************************************************/

#include "TrackScheduler.h"

#include "Types/GlobalTypes.h"

using namespace std;

TrackScheduler::TrackScheduler(DataStore* const datastore,
                               MusicBackend* const musicbackend) {
  if (datastore == nullptr)
    LOG(ERROR) << "TrackScheduler Ctor: datastore is nullptr!";
  if (musicbackend == nullptr)
    LOG(ERROR) << "TrackScheduler Ctor: datastore is nullptr!";

  mDataStore = datastore;
  mMusicBackend = musicbackend;
}

TrackScheduler::~TrackScheduler() {
  mThread.join();

  /* Memory needs to be freed by the creator of this object.
   * Just de-initializing the pointers here for safety reasons.
   */
  mDataStore = nullptr;
  mMusicBackend = nullptr;
}

bool TrackScheduler::start() {
  mThread = thread(&TrackScheduler::doSchedule, this);
  return true;
}

bool TrackScheduler::doSchedule() {
  unsigned counter = 0;
  while (counter < 10) {
    LOG(INFO) << "Hello World Scheduler [" << counter++ << "]";
    this_thread::sleep_for(chrono::milliseconds(100));
  }

  if (mDataStore == nullptr || mMusicBackend == nullptr) {
    LOG(ERROR) << "TaskScheduler.doSchedule: nullptr";
    return false;
  }

  /* Get current track from DataStore */
  auto retStore = mDataStore->getPlayingTrack();
  if (checkAlternativeError(retStore))
    return false;
  QueuedTrack trkStore = get<QueuedTrack>(retStore);

  /* Get current track from Spotify */
  auto retSpotify = mMusicBackend->getCurrentPlayback();
  if (checkAlternativeError(retSpotify))
    return false;
  PlaybackTrack trkSptfy = get<PlaybackTrack>(retSpotify);

  /* Check consistency */
  if (!(trkStore == trkSptfy)) {
    string msg =
        "TrackScheduler.doSchedule: Inconsistency between "
        "current playback track in Spotify and DataStore";
    LOG(ERROR) << msg;
    return false;
  }

  /* Check playback progress and sleep.
   * Wakeup with a margin to the end of the track, so be able to set
   * the next track in time.
   */
  if (trkSptfy.progressMs < trkSptfy.durationMs) {
    const unsigned setNewTrackMarginMs = 1000;
    unsigned sleepMs =
        trkSptfy.durationMs - trkSptfy.progressMs - setNewTrackMarginMs;
    this_thread::sleep_for(chrono::milliseconds(sleepMs));
  }

  /* Advance playlist in DataStore */
  auto ret = mDataStore->nextTrack();
  if (checkOptionalError(ret))
    return false;

  /* Get next track */
  auto nextTrk = mDataStore->getPlayingTrack();
  if (checkAlternativeError(nextTrk))
    return false;

  /* Play the track */
  ret = mMusicBackend->setPlayback(get<QueuedTrack>(nextTrk));
  if (checkOptionalError(ret))
    return false;

  return true;
}