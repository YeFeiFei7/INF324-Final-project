
package com.Cartoonifier;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.Window;

public class CartoonifierApp implements OnTouchListener {
    private static final String TAG = "CartoonifierApp";
    private CartoonifierView mView;

    private MenuItem mMenuSketch;
    private MenuItem mMenuAlien;
    private MenuItem mMenuEvil;
    private MenuItem mMenuDebug;

    public CartoonifierApp() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
        mView.releaseCamera();
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
        if( !mView.openCamera() ) {
            AlertDialog ad = new AlertDialog.Builder(this).create();  
            ad.setCancelable(false);   
            ad.setMessage("Fatal error: can't open camera!");  
            ad.setButton("OK", new DialogInterface.OnClickListener() {  
                public void onClick(DialogInterface dialog, int which) {  
                    dialog.dismiss();                      
                    finish();
                }  
            });  
            ad.show();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        mView = new CartoonifierView(this);
        setContentView(mView);

        mView.setOnTouchListener(this);
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "onCreateOptionsMenu");
        mMenuSketch = menu.add("Sketch or Painting");
        mMenuAlien = menu.add("Alien or Human");
        mMenuEvil = menu.add("Evil or Good");
        mMenuDebug = menu.add("[Debug mode]");
        return true;
    }


    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "Menu Item selected: " + item);
        if (item == mMenuSketch)
            mView.toggleSketchMode();
        else if (item == mMenuAlien)
            mView.toggleAlienMode();
        else if (item == mMenuEvil)
            mView.toggleEvilMode();
        else if (item == mMenuDebug)
            mView.toggleDebugMode();
        return true;
    }
    

    public boolean onTouch(View v, MotionEvent m) {

        if (m.getAction() != MotionEvent.ACTION_DOWN) {
            return false;
        }
        
        Log.i(TAG, "onTouch down event");
        mView.nextFrameShouldBeSaved(getBaseContext());

        return false;
    }
}
