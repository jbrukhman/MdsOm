package com.jpmorgan.mds.mercury.helper;

import com.wombat.mama.MamaTransport;

public interface MdsOmTransportCallback {
    /*
      Invoked when the transport connects 
     
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport which has connected.
      @param platformInfo Info associated with the event.
     
      The cause and platformInfo are supplied only by some middlewares.
      The information provided by platformInfo is middleware specific.
      The following middlewares are supported:
     
      tibrv: provides the char version of the tibrv advisory message.
      wmw:   provides a pointer to a C mamaConnection struct for the event
     */
	public void onDisconnect(
		MdsOmEnv env,
		MamaTransport transport,
		Object platformInfo);

   /*
      Invoked when the transport reconnects 
     
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport which has reconnected.
      @param platformInfo Info associated with the event.
     
      The cause and platformInfo are supplied only by some middlewares.
      The information provided by platformInfo is middleware specific.
      The following middlewares are supported:
     
      tibrv: provides the char version of the tibrv advisory message.
      wmw:   provides a pointer to a C mamaConnection struct for the event
     */
	public void onReconnect(
		MdsOmEnv env,
		MamaTransport transport,
		Object platformInfo);

   /*
      Invoked when the quality of this transport changes.
     
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport on which the quality has changed.
      @param cause The cause of the quality event.
      @param platformInfo Info associated with the quality event.
     
      The cause and platformInfo are supplied only by some middlewares.
      The information provided by platformInfo is middleware specific.
      The following middlewares are supported:
     
      tibrv: provides the char version of the tibrv advisory message.
     */
	public void onQuality(
		MdsOmEnv env,
       MamaTransport     transport,
       short              cause,
       Object        platformInfo);

   /*
      Invoked on the subscriber when the transport connects.
      
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport which has connected.
      @param platformInfo Info associated with the event.
     
      The cause and platformInfo are supplied only by some middlewares.
      The information provided by platformInfo is middleware specific.
      The following middlewares are supported:
     
      wmw:   provides a pointer to a C mamaConnection struct for the event
     */
	public void onConnect(
		MdsOmEnv env,
       MamaTransport  transport,
       Object     platformInfo);

   /*
      Invoked on the publisher when the transport accepts a connection.
     
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport which has accepted.
      @param platformInfo Info associated with the event.
     
      The cause and platformInfo are supplied only by some middlewares.
      The information provided by platformInfo is middleware specific.
      The following middlewares are supported:
     
      wmw:   provides a pointer to a C mamaConnection struct for the event
     */
   void onAccept(
		MdsOmEnv env,
       MamaTransport   transport,
       Object      platformInfo);

	/*
      Invoked on the publisher when the transport accepts a reconnection.
     
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport which has reconnected on
      @param platformInfo Info associated with the event.
     
      The cause and platformInfo are supplied only by some middlewares.
      The information provided by platformInfo is middleware specific.
      The following middlewares are supported:
     
      wmw:   provides a pointer to a C mamaConnection struct for the event
     */
   void onAcceptReconnect (
		MdsOmEnv env,
       MamaTransport  transport,
       Object     platformInfo);

   /*
      Invoked on the subscriber when the transport disconnects from the publisher.
     
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport which has disconnected on
      @param platformInfo Info associated with the event.
     
      The cause and platformInfo are supplied only by some middlewares.
      The information provided by platformInfo is middleware specific.
      The following middlewares are supported:
     
      wmw:   provides a pointer to a C mamaConnection struct for the event
     */
   void onPublisherDisconnect (
		MdsOmEnv env,
       MamaTransport  transport,
       Object     platformInfo);

   /*
      Invoked on the subscriber when the naming service connects.
      
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport which has connected.
      @param platformInfo Info associated with the event.
     */
   public void onNamingServiceConnect (
		MdsOmEnv env,
       MamaTransport  transport,
       Object     platformInfo);

   /* 
      Invoked on the subscriber when the naming service disconnects.
      
	   @param env The MdsOmEnv that holds the transport.
      @param transport The transport which has connected.
      @param platformInfo Info associated with the event.
     */
   public void onNamingServiceDisconnect (
		MdsOmEnv env,
       MamaTransport  transport,
       Object     platformInfo);
}
