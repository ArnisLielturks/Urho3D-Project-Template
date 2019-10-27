package com.github.urho3d.launcher;

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

    final int ANDROID_AD_INITIALIZED = 10;

    final int ANDROID_AD_LOAD_INTERSTITIAL = 20;
    final int ANDROID_AD_SHOW_INTERSTITIAL = 21;
    final int ANDROID_AD_INTERSTITIAL_LOADED = 22;
    final int ANDROID_AD_SHOW_INTERSTITIAL_OPENED = 23;
    final int ANDROID_AD_INTERSTITIAL_NOT_LOADED = 24;
    final int ANDROID_AD_INTERSTITIAL_CLOSED = 25;

    final int ANDROID_AD_LOAD_REWARDED = 30;
    final int ANDROID_AD_REWARDED_SHOW = 31;
    final int ANDROID_AD_REWARDED_LOADED = 32;
    final int ANDROID_AD_REWARDED_FAILED_TO_LOAD = 33;
    final int ANDROID_AD_REWARDED_OPENED = 34;
    final int ANDROID_AD_REWARDED_CLOSED = 35;
    final int ANDROID_AD_REWARDED_EARNED = 36;
    final int ANDROID_AD_REWARDED_FAILED_TO_SHOW = 37;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MobileAds.initialize(this, "ca-app-pub-3940256099942544~3347511713");
        mInterstitialAd = new InterstitialAd(this);
        mInterstitialAd.setAdUnitId("ca-app-pub-3940256099942544/1033173712");

        MobileAds.initialize(this, new OnInitializationCompleteListener() {
            @Override
            public void onInitializationComplete(InitializationStatus initializationStatus) {
                 SDLActivity.SendServiceCommand(ANDROID_AD_INITIALIZED, 1, "ANDROID_AD_INITIALIZED");
            }
        });

        rewardedAd = new RewardedAd(this, "ca-app-pub-3940256099942544/5224354917");
        
    }

    @Override
    public void onMessageReceivedFromGame(int id)
    {
        super.onMessageReceivedFromGame(id);
        if (id == ANDROID_AD_LOAD_INTERSTITIAL) {
            mInterstitialAd.loadAd(new AdRequest.Builder().build());
            // Set an AdListener.
            mInterstitialAd.setAdListener(new AdListener() {
            @Override
                public void onAdLoaded() {
                    SDLActivity.SendServiceCommand(ANDROID_AD_INTERSTITIAL_LOADED, 1, "ANDROID_AD_INTERSTITIAL_LOADED");
                }

                @Override
                public void onAdClosed() {
                    SDLActivity.SendServiceCommand(ANDROID_AD_INTERSTITIAL_CLOSED, 1, "ANDROID_AD_INTERSTITIAL_CLOSED");
                }
            });
        } else if (id == ANDROID_AD_LOAD_REWARDED) {
            RewardedAdLoadCallback adLoadCallback = new RewardedAdLoadCallback() {
                @Override
                public void onRewardedAdLoaded() {
                    // Ad successfully loaded.
                    SDLActivity.SendServiceCommand(ANDROID_AD_REWARDED_LOADED, 1, "ANDROID_AD_REWARDED_LOADED");
                }

               @Override
                public void onRewardedAdFailedToLoad(int errorCode) {
                    // Ad failed to load.
                    SDLActivity.SendServiceCommand(ANDROID_AD_REWARDED_FAILED_TO_LOAD, 1, "ANDROID_AD_REWARDED_FAILED_TO_LOAD");
                }
            };
            rewardedAd.loadAd(new AdRequest.Builder().build(), adLoadCallback);
        } else if (id == ANDROID_AD_SHOW_INTERSTITIAL) {
            if (mInterstitialAd && mInterstitialAd.isLoaded()) {
                mInterstitialAd.show();
                SDLActivity.SendServiceCommand(ANDROID_AD_SHOW_INTERSTITIAL_OPENED, 1, "ANDROID_AD_SHOW_INTERSTITIAL_OPENED");
            } else {
                SDLActivity.SendServiceCommand(ANDROID_AD_INTERSTITIAL_NOT_LOADED, 1, "ANDROID_AD_INTERSTITIAL_NOT_LOADED");
            }
        } else if (id == ANDROID_AD_REWARDED_SHOW) {
            if (rewardedAd && rewardedAd.isLoaded()) {
                MainActivity activityContext = this;
                RewardedAdCallback adCallback = new RewardedAdCallback() {
                    @Override
                    public void onRewardedAdOpened() {
                        // Ad opened.
                        SDLActivity.SendServiceCommand(ANDROID_AD_REWARDED_OPENED, 1, "ANDROID_AD_REWARDED_OPENED");
                    }

                    @Override
                    public void onRewardedAdClosed() {
                        // Ad closed.
                        SDLActivity.SendServiceCommand(ANDROID_AD_REWARDED_CLOSED, 1, "ANDROID_AD_REWARDED_CLOSED");
                    }

                    @Override
                    public void onUserEarnedReward(@NonNull RewardItem reward) {
                        // User earned reward.
                        SDLActivity.SendServiceCommand(ANDROID_AD_REWARDED_EARNED, 1, "ANDROID_AD_REWARDED_EARNED");
                    }

                    @Override
                    public void onRewardedAdFailedToShow(int errorCode) {
                        // Ad failed to display
                        SDLActivity.SendServiceCommand(ANDROID_AD_REWARDED_FAILED_TO_SHOW, 1, "ANDROID_AD_REWARDED_FAILED_TO_SHOW");
                    }
                };
                rewardedAd.show(activityContext, adCallback);
            } else {
                SDLActivity.SendServiceCommand(ANDROID_AD_REWARDED_FAILED_TO_LOAD, 1, "ANDROID_AD_REWARDED_FAILED_TO_LOAD");
            }
        }
    }
}
