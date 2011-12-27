/**
 * Copyright 2011 Kurtis L. Nusbaum
 * 
 * This file is part of UDJ.
 * 
 * UDJ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * UDJ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with UDJ.  If not, see <http://www.gnu.org/licenses/>.
 */
package org.klnusbaum.udj.network;


import android.content.Context;
import android.content.ContentResolver;
import android.os.Bundle;
import android.accounts.Account;
import android.accounts.AccountManager;
import android.accounts.OperationCanceledException;
import android.accounts.AuthenticatorException;
import android.database.Cursor;
import android.os.RemoteException;
import android.content.OperationApplicationException;
import android.util.Log;
import android.app.IntentService;
import android.content.ContentValues;
import android.net.Uri;
import android.content.Intent;

import java.util.GregorianCalendar;
import java.util.List;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;


import org.json.JSONException;

import org.apache.http.auth.AuthenticationException;
import org.apache.http.ParseException;

import org.klnusbaum.udj.containers.PlaylistEntry;
import org.klnusbaum.udj.Constants;
import org.klnusbaum.udj.UDJEventProvider;


/**
 * Adapter used to sync up with the UDJ server.
 */
public class PlaylistSyncService extends IntentService{

  private static final String TAG = "PlyalistSyncService";
  private static final String[] addRequestsProjection = new String[] {
    UDJEventProvider.ADD_REQUEST_ID_COLUMN,
    UDJEventProvider.ADD_REQUEST_LIB_ID_COLUMN};
  private static final String addRequestSeleciton = 
    UDJEventProvider.ADD_REQUEST_SYNC_STATUS_COLUMN + 
    "=" +
    UDJEventProvider.ADD_REQUEST_NEEDS_SYNC;

  public PlaylistSyncService(){
    super("PlaylistSyncService");
  }

  @Override
  public void onHandleIntent(Intent intent){
    Log.i(TAG, "In playlist sync server");
    final Account account = 
      (Account)intent.getParcelableExtra(Constants.ACCOUNT_EXTRA);
    long eventId = intent.getLongExtra(Constants.EVENT_ID_EXTRA, -1);
    //TODO hanle error if eventId or account aren't provided
    if(intent.getAction().equals(Intent.ACTION_INSERT)){
      syncAddRequests(account, eventId);
    }
    else if(intent.getAction().equals(Intent.ACTION_VIEW)){
      updateActivePlaylist(account, eventId); 
    }
  }

  private void updateActivePlaylist(Account account, long eventId){
    Log.d(TAG, "updating active playlist");
    try{
      String authToken = 
        AccountManager.get(this).blockingGetAuthToken(account, "", true);
      List<PlaylistEntry> newPlaylist =
        ServerConnection.getActivePlaylist(eventId, authToken);
      RESTProcessor.setActivePlaylist(newPlaylist, this);
    }
    catch(JSONException e){
      Log.e(TAG, "JSON exception when retreiving playist");
    }
    catch(ParseException e){
      Log.e(TAG, "Parse exception when retreiving playist");
    }
    catch(IOException e){
      Log.e(TAG, "IO exception when retreiving playist");
    }
    catch(AuthenticationException e){
      Log.e(TAG, "Authentication exception when retreiving playist");
    }
    catch(AuthenticatorException e){
      Log.e(TAG, "Authentication exception when retreiving playist");
    }
    catch(OperationCanceledException e){
      Log.e(TAG, "Op Canceled exception when retreiving playist");
    }
    catch(RemoteException e){
      Log.e(TAG, "Remote exception when retreiving playist");
    }
    catch(OperationApplicationException e){
      Log.e(TAG, "Operation Application exception when retreiving playist");
    }
    //TODO This point of the app seems very dangerous as there are so many
    // exceptions that could occuer. Need to pay special attention to this.
  }

  private void syncAddRequests(Account account, long eventId){
    Log.d(TAG, "Sycning add requests");
    try{
      String authToken = 
        AccountManager.get(this).blockingGetAuthToken(account, "", true);
      ContentResolver cr = getContentResolver();
      Cursor requestsCursor = cr.query(
        UDJEventProvider.PLAYLIST_ADD_REQUEST_URI,
        addRequestsProjection,
        addRequestSeleciton,
        null,
        null);
      HashMap<Long, Long> addRequests = new HashMap<Long, Long>();
      if(requestsCursor.moveToFirst()){
        int requestIdColumn = requestsCursor.getColumnIndex(
          UDJEventProvider.ADD_REQUEST_ID_COLUMN);
        int libIdColumn = requestsCursor.getColumnIndex(
          UDJEventProvider.ADD_REQUEST_LIB_ID_COLUMN);
        do{
          addRequests.put(
            requestsCursor.getLong(requestIdColumn),
            requestsCursor.getLong(libIdColumn)); 
        }while(requestsCursor.moveToNext());
      }
      requestsCursor.close();
      if(addRequests.size() >0){
        ServerConnection.addSongsToActivePlaylist(
          addRequests, eventId, authToken);
        RESTProcessor.setPlaylistAddRequestsSynced(addRequests.keySet(), this);
        updateActivePlaylist(account, eventId);
      }
    }
    catch(JSONException e){
      Log.e(TAG, "JSON exception when retreiving playist");
    }
    catch(ParseException e){
      Log.e(TAG, "Parse exception when retreiving playist");
    }
    catch(IOException e){
      Log.e(TAG, "IO exception when retreiving playist");
    }
    catch(AuthenticationException e){
      Log.e(TAG, "Authentication exception when retreiving playist");
    }
    catch(AuthenticatorException e){
      Log.e(TAG, "Authentication exception when retreiving playist");
    }
    catch(OperationCanceledException e){
      Log.e(TAG, "Op Canceled exception when retreiving playist");
    }
    catch(RemoteException e){
      Log.e(TAG, "Remote exception when retreiving playist");
    }
    catch(OperationApplicationException e){
      Log.e(TAG, "Operation Application exception when retreiving playist");
    }
    //TODO This point of the app seems very dangerous as there are so many
    // exceptions that could occuer. Need to pay special attention to this.
  }

}