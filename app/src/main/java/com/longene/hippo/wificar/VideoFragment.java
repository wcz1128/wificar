package com.longene.hippo.wificar;

import android.app.Fragment;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.Toast;

/**
 * Created by hippo on 2017/8/15.
 */

public class VideoFragment extends Fragment {
    GL2JNIView mzhuaziview;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState)
    {
        ViewGroup view=  (ViewGroup)inflater.inflate(R.layout.video_content, container, false);
        SetToast("进入控制面板");
        mzhuaziview = new GL2JNIView(((MainActivity)getActivity()).getApplication());
        mzhuaziview.setzhuaziRender();
        FrameLayout.LayoutParams zhuaziFL = new FrameLayout.LayoutParams(640, 480, Gravity.RIGHT|Gravity.TOP);//W H  右上角
        zhuaziFL.setMargins(0, 0, 0, 0);  // 设置内边距，如果占满屏幕就可以指定位置了
        mzhuaziview.setLayoutParams(zhuaziFL);

        view.addView(mzhuaziview);
        Log.d("hippo","onCreateView " + mzhuaziview);
        return view;
    }

    public void SetToast(CharSequence msg){
        Toast mToast = ((MainActivity)getActivity()).getToast();
        if(mToast == null) {
            Log.d("hippo","Toast is null in click");
        }else {
            mToast.setText(msg);
            mToast.show();
        }
    }

    @Override public void onPause() {
        super.onPause();
        Log.d("hippo","ContolFragment onPause");
        mzhuaziview.onPause();
    }

    @Override public void onResume() {
        super.onResume();
        Log.d("hippo","ContolFragment onResume");
        mzhuaziview.onResume();
    }

}
