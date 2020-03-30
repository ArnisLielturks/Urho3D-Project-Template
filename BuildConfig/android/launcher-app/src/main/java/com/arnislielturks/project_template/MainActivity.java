package com.github.arnislielturks.project_template;

import org.libsdl.app.SDLActivity;
import android.os.Bundle;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.*;
import androidx.annotation.NonNull;
import com.google.android.gms.ads.initialization.*;
import com.google.android.gms.ads.rewarded.*;

public class MainActivity extends SDLActivity {

    private InterstitialAd mInterstitialAd;

    private RewardedAd rewardedAd;

    public static final int ANDROID_AD_INITIALIZED = 10;

    public static final int ANDROID_AD_LOAD_INTERSTITIAL = 20;
    public static final int ANDROID_AD_SHOW_INTERSTITIAL = 21;
    public static final int ANDROID_AD_INTERSTITIAL_LOADED = 22;
    public static final int ANDROID_AD_SHOW_INTERSTITIAL_OPENED = 23;
    public static final int ANDROID_AD_INTERSTITIAL_NOT_LOADED = 24;
    public static final int ANDROID_AD_INTERSTITIAL_CLOSED = 25;

    public static final int ANDROID_AD_LOAD_REWARDED = 30;
    public static final int ANDROID_AD_REWARDED_SHOW = 31;
    public static final int ANDROID_AD_REWARDED_LOADED = 32;
    public static final int ANDROID_AD_REWARDED_FAILED_TO_LOAD = 33;
    public static final int ANDROID_AD_REWARDED_OPENED = 34;
    public static final int ANDROID_AD_REWARDED_CLOSED = 35;
    public static final int ANDROID_AD_REWARDED_EARNED = 36;
    public static final int ANDROID_AD_REWARDED_FAILED_TO_SHOW = 37;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MobileAds.initialize(this, "ca-app-pub-3940256099942544~3347511713");
        MobileAds.initialize(this, new OnInitializationCompleteListener() {
            @Override
            public void onInitializationComplete(InitializationStatus initializationStatus) {
                 SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_INITIALIZED, 1, "ANDROID_AD_INITIALIZED");
            }
        });
        
    }

    @Override
    public void onMessageReceivedFromGame(int id)
    {
        super.onMessageReceivedFromGame(id);
        if (id == MainActivity.ANDROID_AD_LOAD_INTERSTITIAL) {
            mInterstitialAd = new InterstitialAd(this);
            mInterstitialAd.setAdUnitId("ca-app-pub-3940256099942544/1033173712");
            mInterstitialAd.loadAd(new AdRequest.Builder().build());
            // Set an AdListener.
            mInterstitialAd.setAdListener(new AdListener() {
            @Override
                public void onAdLoaded() {
                    SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_INTERSTITIAL_LOADED, 1, "ANDROID_AD_INTERSTITIAL_LOADED");
                }

                @Override
                public void onAdClosed() {
                    SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_INTERSTITIAL_CLOSED, 1, "ANDROID_AD_INTERSTITIAL_CLOSED");
                }
            });
        } else if (id == MainActivity.ANDROID_AD_LOAD_REWARDED) {
            rewardedAd = new RewardedAd(this, "ca-app-pub-3940256099942544/5224354917");
            RewardedAdLoadCallback adLoadCallback = new RewardedAdLoadCallback() {
                @Override
                public void onRewardedAdLoaded() {
                    // Ad successfully loaded.
                    SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_REWARDED_LOADED, 1, "ANDROID_AD_REWARDED_LOADED");
                }

               @Override
                public void onRewardedAdFailedToLoad(int errorCode) {
                    // Ad failed to load.
                    SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_REWARDED_FAILED_TO_LOAD, 1, "ANDROID_AD_REWARDED_FAILED_TO_LOAD");
                }
            };
            rewardedAd.loadAd(new AdRequest.Builder().build(), adLoadCallback);
        } else if (id == MainActivity.ANDROID_AD_SHOW_INTERSTITIAL) {
            if (mInterstitialAd != null && mInterstitialAd.isLoaded()) {
                mInterstitialAd.show();
                SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_SHOW_INTERSTITIAL_OPENED, 1, "ANDROID_AD_SHOW_INTERSTITIAL_OPENED");
            } else {
                SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_INTERSTITIAL_NOT_LOADED, 1, "ANDROID_AD_INTERSTITIAL_NOT_LOADED");
            }
        } else if (id == MainActivity.ANDROID_AD_REWARDED_SHOW) {
            if (rewardedAd != null && rewardedAd.isLoaded()) {
                MainActivity activityContext = this;
                RewardedAdCallback adCallback = new RewardedAdCallback() {
                    @Override
                    public void onRewardedAdOpened() {
                        // Ad opened.
                        SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_REWARDED_OPENED, 1, "ANDROID_AD_REWARDED_OPENED");
                    }

                    @Override
                    public void onRewardedAdClosed() {
                        // Ad closed.
                        SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_REWARDED_CLOSED, 1, "ANDROID_AD_REWARDED_CLOSED");
                    }

                    @Override
                    public void onUserEarnedReward(@NonNull RewardItem reward) {
                        // User earned reward.
                        SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_REWARDED_EARNED, 1, "ANDROID_AD_REWARDED_EARNED");
                    }

                    @Override
                    public void onRewardedAdFailedToShow(int errorCode) {
                        // Ad failed to display
                        SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_REWARDED_FAILED_TO_SHOW, 1, "ANDROID_AD_REWARDED_FAILED_TO_SHOW");
                    }
                };
                rewardedAd.show(activityContext, adCallback);
            } else {
                SDLActivity.SendServiceCommand(MainActivity.ANDROID_AD_REWARDED_FAILED_TO_LOAD, 1, "ANDROID_AD_REWARDED_FAILED_TO_LOAD");
            }
        }
    }
}
