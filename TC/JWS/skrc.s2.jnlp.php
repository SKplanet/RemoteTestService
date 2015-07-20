<?php
    header('Content-Type: application/x-java-jnlp-file');
    header("Content-Disposition: attachment; filename=skrc.s2.jnlp");
    $reservID = $_GET['reservation-id'];
    $reservIP = $_GET['reservation-ip'];
    $reservPT = $_GET['reservation-pt'];
    echo("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
?>
<jnlp spec="1.0+" codebase="http://121.140.140.35:9685" href="skrc.s2.jnlp.php<?
    if( $reservID === null || strlen($reservID) <= 0 ) {
	    echo("\">");
    } else {
	    echo("?reservation-id=".$reservID);
	    if( $reservIP != null && strlen($reservIP) > 0 ) {
		    echo("&reservation-ip=".$reservIP);
	    }
	    if( $reservPT != null && strlen($reservPT) > 0 ) {
		    echo("&reservation-pt=".$reservPT);
	    }
	    echo( "\">");
    }
?>
    <information>
        <title>Remote Test</title>
        <vendor>SK Planet</vendor>
        <description>Remote Test Client Program</description>
        <description kind="short">Remote Test</description>
        <icon href="tc.png"/>
    </information>
    <resources>
        <!-- Application Resources -->
        <j2se version="1.5+"
              href="http://java.sun.com/products/autodl/j2se"/>
        <jar href="skrc.s2.jar"
            main="true" />

    </resources>
    <security>
        <all-permissions/>
    </security>
    <application-desc
         name="Remote Test Client Application"
         main-class= "com.skplanet.skrc2.main.TCFrame"
         width="480"
         height="800">
<?
    if( $reservID != null || strlen($reservID) > 0 ) {
	    echo("<argument>".$reservID."</argument>");
    }
    if( $reservIP != null && strlen($reservIP) > 0 ) {
	    echo("<argument>".$reservIP."</argument>");
    }
    if( $reservPT != null && strlen($reservPT) > 0 ) {
	    echo("<argument>".$reservPT."</argument>");
    }
?>
     </application-desc>
     <update check="always" policy="prompt-update">
</jnlp>
