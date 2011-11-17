from django.conf.urls.defaults import patterns, include, url

urlpatterns = patterns('udj.auth',
    (r'^auth/?$', 'authenticate'),
)

urlpatterns += patterns('udj.views.library',
  (r'^users/(?P<user_id>\d+)/library/songs$', 'addSongsToLibrary'),
  (
    r'^users/(?P<user_id>\d+)/library/(?P<lib_id>\d+)$', 
    'deleteSongFromLibrary'
  ),
  (r'^users/(?P<user_id>\d+)/library$', 'deleteEntireLibrary'),
) 

urlpatterns += patterns('udj.views.event',
  (r'^event/(?P<latitude>-?\d+\.\d+)/(?P<longitude>-?\d+\.\d+)', 'getNearbyEvents'),
)

"""

urlpatterns += patterns('udj.views.playlist',
  (r'^users/(?P<user_id>\d+)/playlists$', 'addPlaylists'),
  (
    r'^users/(?P<user_id>\d+)/playlists/(?P<playlist_id>\d+)$', 
    'deletePlaylist'
  ),
  (
     r'^users/(?P<user_id>\d+)/playlists/(?P<playlist_id>\d+)/songs$', 
    'addPlaylistEntries'
  ),
  (
    r'^users/(?P<user_id>\d+)/playlists/(?P<playlist_id>\d+)/' 
      'songs/(?P<playlist_entry_id>\d+)$', 
    'deleteSongFromPlaylist'
  ),
) 
"""
