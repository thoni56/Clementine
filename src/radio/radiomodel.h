/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RADIOMODEL_H
#define RADIOMODEL_H

#include "core/backgroundthread.h"
#include "core/song.h"
#include "playlist/playlistitem.h"
#include "ui/settingsdialog.h"
#include "widgets/multiloadingindicator.h"

class Database;
class MergedProxyModel;
class RadioService;
class SettingsDialog;
class TaskManager;

#ifdef HAVE_LIBLASTFM
  class LastFMService;
#endif

class RadioModel : public QStandardItemModel {
  Q_OBJECT

public:
  RadioModel(BackgroundThread<Database>* db_thread, TaskManager* task_manager,
             QObject* parent = 0);

  enum Role {
    // Services can use this role to distinguish between different types of
    // items that they add.  The root item's type is automatically set to
    // Type_Service, but apart from that Services are free to define their own
    // values for this field (starting from TypeCount).
    Role_Type = Qt::UserRole + 1000,

    // If this is not set the item is not playable (ie. it can't be dragged to
    // the playlist).  Otherwise it describes how this item is converted to
    // playlist items.  See the PlayBehaviour enum for more details.
    Role_PlayBehaviour,

    // The URL of the media for this item.  This is required if the
    // PlayBehaviour is set to something other than None.  How the URL is
    // used depends on the PlayBehaviour.
    Role_Url,

    // These fields are used to populate the playlist columns for this item
    // only when using PlayBehaviour_SingleItem.  They are ignored otherwise
    Role_Title,
    Role_Artist,

    // If this is set to true then the model will call the service's
    // LazyPopulate method when this item is expanded.  Use this if your item's
    // children have to be downloaded or fetched remotely.
    Role_CanLazyLoad,

    // This is automatically set on the root item for a service.  It contains
    // a pointer to a RadioService.  Services should not set this field
    // themselves.
    Role_Service,
  };

  enum Type {
    Type_Service = 1,

    TypeCount
  };

  enum PlayBehaviour {
    // The item can't be played.  This is the default.
    PlayBehaviour_None = 0,

    // This item's URL is passed through the normal song loader.  This supports
    // loading remote playlists, remote files and local files.  This is probably
    // the most sensible behaviour to use if you're just returning normal radio
    // stations.
    PlayBehaviour_UseSongLoader,

    // This item's URL, Title and Artist are used in the playlist.  No special
    // behaviour occurs - the URL is just passed straight to gstreamer when
    // the user starts playing.
    PlayBehaviour_SingleItem,
  };

  // Needs to be static for RadioPlaylistItem::restore
  static RadioService* ServiceByName(const QString& name);

  template<typename T>
  static T* Service() {
    return static_cast<T*>(ServiceByName(T::kServiceName));
  }

  RadioService* ServiceForItem(const QStandardItem* item) const;
  RadioService* ServiceForIndex(const QModelIndex& index) const;

  bool IsPlayable(const QModelIndex& index) const;

  // This is special because Player needs it for scrobbling
#ifdef HAVE_LIBLASTFM
  LastFMService* GetLastFMService() const;
#endif

  // QAbstractItemModel
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  bool hasChildren(const QModelIndex& parent) const;
  int rowCount(const QModelIndex& parent) const;

  void ShowContextMenu(const QModelIndex& merged_model_index,
                       const QPoint& global_pos);
  void ReloadSettings();

  BackgroundThread<Database>* db_thread() const { return db_thread_; }
  MergedProxyModel* merged_model() const { return merged_model_; }
  TaskManager* task_manager() const { return task_manager_; }

signals:
  void AsyncLoadFinished(const PlaylistItem::SpecialLoadResult& result);
  void StreamError(const QString& message);
  void StreamMetadataFound(const QUrl& original_url, const Song& song);
  void OpenSettingsAtPage(SettingsDialog::Page);

  /*void AddItemToPlaylist(RadioItem* item, bool clear_first);*/
  void AddItemsToPlaylist(const PlaylistItemList& items, bool clear_first);

private:
  void AddService(RadioService* service);

private:
  static QMap<QString, RadioService*>* sServices;
  BackgroundThread<Database>* db_thread_;
  MergedProxyModel* merged_model_;
  TaskManager* task_manager_;
};

#endif // RADIOMODEL_H
