package com.github.urho3d.launcher;

import org.libsdl.app.SDLActivity;
import android.os.Bundle;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdCallback;
import com.google.android.gms.ads.rewarded.RewardItem;

public class MainActivity extends SDLActivity {

    private InterstitialAd mInterstitialAd;

    private RewardedAd rewardedAd;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MobileAds.initialize(this,
            "ca-app-pub-3940256099942544~3347511713");
        mInterstitialAd = new InterstitialAd(this);
        mInterstitialAd.setAdUnitId("ca-app-pub-3940256099942544/1033173712");
        mInterstitialAd.loadAd(new AdRequest.Builder().build());
        MobileAds.initialize(this, new OnInitializationCompleteListener() {
            @Override
            public void onInitializationComplete(InitializationStatus initializationStatus) {
                 SDLActivity.SendServiceCommand(id, 1, "Ads initialized!");
            }
        });

        rewardedAd = new RewardedAd(this,
            "ca-app-pub-3940256099942544/5224354917");

        RewardedAdLoadCallback adLoadCallback = new RewardedAdLoadCallback() {
            @Override
            public void onRewardedAdLoaded() {
                // Ad successfully loaded.
            }

           @Override
            public void onRewardedAdFailedToLoad(int errorCode) {
                // Ad failed to load.
            }
        };
        rewardedAd.loadAd(new AdRequest.Builder().build(), adLoadCallback);
    }

    @Override
    public void onMessageReceivedFromGame(int id)
    {
        if (id == 10) {
            super.onMessageReceivedFromGame(id);
            SDLActivity.SendServiceCommand(id, 1, "This message was triggered from MainActivity!");
            if (mInterstitialAd.isLoaded()) {
                    mInterstitialAd.show();
            } else {
                SDLActivity.SendServiceCommand(id, 1, "AD not yet loaded");
            }
        } else if (id == 11) {
            if (rewardedAd.isLoaded()) {
                MainActivity activityContext = this;
                RewardedAdCallback adCallback = new RewardedAdCallback() {
                    @Override
                    public void onRewardedAdOpened() {
                        // Ad opened.
                    }

                    @Override
                    public void onRewardedAdClosed() {
                        // Ad closed.
                    }

                    @Override
                    public void onUserEarnedReward(RewardItem reward) {
                        // User earned reward.
                        SDLActivity.SendServiceCommand(id, 1, "Reward earned!!");
                    }

                    @Override
                    public void onRewardedAdFailedToShow(int errorCode) {
                        // Ad failed to display
                    }
                };
                rewardedAd.show(activityContext, adCallback);
            } else {
                SDLActivity.SendServiceCommand(id, 1, "Rewarded ad not loaded yet!");
            }
        }
    }
}
