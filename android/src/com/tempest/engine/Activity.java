package com.tempest.engine;

import android.content.Context;
import android.os.Bundle;
import android.view.Surface;
import android.view.Window;
import android.view.inputmethod.InputMethodManager;

import java.util.ArrayList;

public class Activity extends android.app.Activity {
  private static final ArrayList<Activity> nonAttached=new ArrayList<>();
  private WindowSurface surface;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    this.requestWindowFeature(Window.FEATURE_NO_TITLE);

    super.onCreate(savedInstanceState);
    surface=new WindowSurface(this);
    setContentView(surface);
    surface.requestFocus();

    synchronized(nonAttached){
      nonAttached.add(this);
      }
    }

  @Override
  protected void onDestroy(){
    if(surface!=null)
      ;//surface.nativeCloseEvent();
    super.onDestroy();
    }

  @SuppressWarnings("unused")
  private Surface nativeSurface(){
    return surface.getSurface();
    }

  @SuppressWarnings("unused")
  private static Activity nativeNewWindow(){
    synchronized(nonAttached){
      if(nonAttached.size()>0){
        return nonAttached.remove(nonAttached.size()-1);
        }
      sleep(1);
      }
    return null;
    }

  @SuppressWarnings("unused")
  private static void nativeDelWindow(Activity a){
    synchronized(nonAttached){
      a.finish();
      }
    }

  private static void sleep(int delay){
    try {
      Thread.sleep(delay);
      } catch (InterruptedException e) {
      //---
      }
    }
  
  @SuppressWarnings("unused")
  private void toggleSoftInput(){
    surface.toggleSoftInput();
  }
  @SuppressWarnings("unused")
  private void hideSoftInput(){
    surface.hideSoftInput();
    
  }
  @SuppressWarnings("unused")
  private void showSoftInput(){
    surface.showSoftInput();
  }
}
