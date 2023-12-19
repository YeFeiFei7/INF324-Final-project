
 package com.Cartoonifier;

import android.content.Context;
import android.graphics.Bitmap;

// For saving Bitmaps to file and the Android picture gallery.
import android.graphics.Bitmap.CompressFormat;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.text.format.DateFormat;
import android.util.Log;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

// For showing a Notification message when saving a file.
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ContentValues;
import android.content.Intent;


class CartoonifierView extends CartoonifierViewBase {
    private static final String TAG = "CartoonifierView";
    
    private int mFrameSize;
    private Bitmap mBitmap;
    private int[] mRGBA;


    private boolean m_sketchMode = false;

    private boolean m_alienMode = false;

    private boolean m_evilMode = false;
    private boolean m_debugMode = false;

    private boolean bSaveThisFrame = false;

    private boolean bFreezeOutput = false;

    private static final int FREEZE_OUTPUT_MSECS = 3000;

    private Context mContext;
    
    private int mNotificationID = 0;
    
    
    
    public CartoonifierView(Context context) {
        super(context);
    }

    @Override
    protected void onPreviewStarted(int previewWidth, int previewHeight) {
        mFrameSize = previewWidth * previewHeight;
        mRGBA = new int[mFrameSize];
        mBitmap = Bitmap.createBitmap(previewWidth, previewHeight, Bitmap.Config.ARGB_8888);
    }

    @Override
    protected void onPreviewStopped() {
        if(mBitmap != null) {
            mBitmap.recycle();
            mBitmap = null;
        }
        mRGBA = null;
    }

    protected void showNotificationMessage(Context context, String filename)
    {

        
        final NotificationManager mgr = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);

        if (mNotificationID > 0)
            mgr.cancel(mNotificationID);
        mNotificationID++;
    
        Notification notification = new Notification(R.drawable.icon, "Saving to gallery (image " + mNotificationID + ") ...", System.currentTimeMillis());
        Intent intent = new Intent(context, CartoonifierView.class);
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, intent, 0);
        notification.setLatestEventInfo(context, "Cartoonifier saved " + mNotificationID + " images to Gallery", "Saved as '" + filename + "'", pendingIntent);
        mgr.notify(mNotificationID, notification);
    }


    protected void savePNGImageToGallery(Bitmap bmp, Context context, String baseFilename)
    {
        try {

            String baseFolder = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).getAbsolutePath() + "/";
            File file = new File(baseFolder + baseFilename);
            Log.i(TAG, "Saving the processed image to file [" + file.getAbsolutePath() + "]");
        

            OutputStream out = new BufferedOutputStream(new FileOutputStream(file));

            bmp.compress(CompressFormat.PNG, 100, out);
            out.flush();
            out.close();

            ContentValues image = new ContentValues();
            image.put(Images.Media.TITLE, baseFilename);
            image.put(Images.Media.DISPLAY_NAME, baseFilename);
            image.put(Images.Media.DESCRIPTION, "Processed by the Cartoonifier App");
            image.put(Images.Media.DATE_TAKEN, System.currentTimeMillis());
            image.put(Images.Media.MIME_TYPE, "image/png");
            image.put(Images.Media.ORIENTATION, 0);
            image.put(Images.Media.DATA, file.getAbsolutePath());
            Uri result = context.getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, image);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    
    @Override
    protected Bitmap processFrame(byte[] data) {
        int[] rgba = mRGBA;

        if (bFreezeOutput) {

            bFreezeOutput = false;

            try {
                wait(FREEZE_OUTPUT_MSECS);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            return null;
        }

        String baseFilename = "";
        if (!bSaveThisFrame) {

            if (m_sketchMode) {

                CartoonifyImage(getFrameWidth(), getFrameHeight(), data, rgba, m_sketchMode, m_alienMode, m_evilMode, m_debugMode);
            }
            else {

                ShowPreview(getFrameWidth(), getFrameHeight(), data, rgba);
            }
        }
        else {

            bSaveThisFrame = false;

            bFreezeOutput = true;

            SimpleDateFormat s = new SimpleDateFormat("yyyy-MM-dd,HH-mm-ss");
            String timestamp = s.format(new Date());
            baseFilename = "Cartoon" + timestamp + ".png";

            showNotificationMessage(mContext, baseFilename);

            CartoonifyImage(getFrameWidth(), getFrameHeight(), data, rgba, m_sketchMode, m_alienMode, m_evilMode, m_debugMode);
        }

        Bitmap bmp = mBitmap;
        bmp.setPixels(rgba, 0, getFrameWidth() , 0, 0, getFrameWidth(), getFrameHeight());
        
        if (bFreezeOutput) {

            savePNGImageToGallery(bmp, mContext, baseFilename);
        }
        
        return bmp;
    }
    
    protected void toggleSketchMode() {
        m_sketchMode = !m_sketchMode;
    }
    protected void toggleAlienMode() {
        m_alienMode = !m_alienMode;
    }
    protected void toggleEvilMode() {
        m_evilMode = !m_evilMode;
    }
    protected void toggleDebugMode() {
        m_debugMode = !m_debugMode;
    }

    protected void nextFrameShouldBeSaved(Context context) {
        bSaveThisFrame = true;
        mContext = context;
    }

    public native void ShowPreview(int width, int height, byte[] yuv, int[] rgba);

    public native void CartoonifyImage(int width, int height, byte[] yuv, int[] rgba, boolean sketchMode, boolean alienMode, boolean evilMode, boolean debugMode);

    static {
        System.loadLibrary("cartoonifier");
    }
}
