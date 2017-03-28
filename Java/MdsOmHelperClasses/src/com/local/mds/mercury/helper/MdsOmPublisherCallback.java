package com.jpmorgan.mds.mercury.helper;

public interface MdsOmPublisherCallback {
	public void onPublishError(MdsOmPublisher pub, short status, Object closure);
	public void onPublishCreate(MdsOmPublisher pub, Object closure);
	public void onPublishDestroy(MdsOmPublisher pub, Object closure);
}

