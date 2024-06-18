package com.example.playermetadataretriever;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;

import java.util.Map;

import androidx.fragment.app.FragmentActivity;

/**
 * Loads {@link PlaybackVideoFragment}.
 */
public class PlaybackActivity extends FragmentActivity {

    private  Thread playbackThread = null;
    private static final String TAG = "jiaming PlaybackActivity";

    static {
        System.loadLibrary("playermetadataretriever");
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        if (savedInstanceState == null) {
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(android.R.id.content, new PlaybackVideoFragment())
                    .commit();
        }
    }


    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        if (playbackThread == null) {
            playbackThread = new Thread(runnable);
            playbackThread.start();
        }
    }

    @Override
    protected void onResume(){
        super.onResume();
        Log.d(TAG, "onResume");
    }

    @Override
    protected void onPause(){
        super.onPause();
        Log.d(TAG, "onPause");
    }

    protected void onStop() {
        super.onStop();
        Log.d(TAG, "onStop");
        releaseSource();
    }

    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        releaseSource();
    }

    protected void onRestart() {
        super.onRestart();
        Log.d(TAG, "onRestart");
    }


   private Runnable runnable = new Runnable() {
       @Override
       public void run() {
           int ret = 0 ;
           Log.d(TAG, " PlaybackActivity " );
           ret =  setDataSource("/data/1080.mp4",this); // "/data/guyongzhe.mp3"  "/data/1080.mp4" "/data/dukou.wav"
           if (ret != 0 ) {
               Log.e(TAG, " setDataSource fail" );
               return ;
           }

           Map<String, String> metadata = fetchMetadata();
           for(String key :metadata.keySet()) {
               Log.d(TAG, " metadata key: " + key+ " value:" +metadata.get(key));
           }

           String value = getKeyAlbumInfo("artist");
           if (value != null) {
               Log.d(TAG, " metadata artist value: " + value);
           }

           extractAlbumArt();

           String only_audio = metadata.get("only_audio");
           Log.d(TAG, " metadata only_audio :" + only_audio);
           int isaudio = Integer.parseInt(only_audio);
           if (isaudio != 1) {
               getFrameAtTime(0, 0);
           }

       }
   };

    public void onAlbumArtExtracted(byte[] artData) {
        Log.d(TAG,"onAlbumArtExtracted artData: " + artData.length);
        if (artData.length > 0) {
            Bitmap bitmap = BitmapFactory.decodeByteArray(artData, 0, artData.length);
            //imageView.setImageBitmap(bitmap);
        } else {
            Log.d(TAG,"onAlbumArtExtracted is not Album Art");
        }

    }

    public Map<String, String> fetchMetadata(){
        return getMetadata();
    }

    public native int setDataSource(String url,Object handle);  // set stream url
    public native int getFrameAtTime(long timeUs, int option);

    public native Map<String, String> getMetadata(); // get metadata info

    public native String getKeyAlbumInfo(String name);

    public native void extractAlbumArt();    // albumart pic

    public native void releaseSource();
}