
/**
 * This simple class subsribes for messages, 
 * prints the received messages to standard output 
 * and writes them also to working directory
 * @author Raimondas Sasnauskas <sasnausk@informatik.uni-tuebingen.de>
 */

import org.xmlBlaster.util.Global;
import org.xmlBlaster.client.qos.ConnectQos;
import org.xmlBlaster.client.I_Callback;
import org.xmlBlaster.client.key.UpdateKey;
import org.xmlBlaster.client.qos.UpdateQos;
import org.xmlBlaster.client.I_XmlBlasterAccess;
import org.xmlBlaster.util.MsgUnit;

import java.io.FileWriter;
import java.io.IOException;

public class MessageReceiver implements I_Callback
{
  private static int i = 0;
  
  public MessageReceiver(final Global glob, String topic) {
    try {
      I_XmlBlasterAccess comm = glob.getXmlBlasterAccess();
      ConnectQos qos = new ConnectQos(glob);
      comm.connect(qos, this);
		
      comm.subscribe("<key oid='" + topic + "'/>", "<qos/>");
      Global.waitOnKeyboardHit("\nHit a key to logout and terminate ...");

      comm.erase("<key oid='" + topic + "'/>", null);
      comm.disconnect(null);
    } catch (Exception e) {
      System.err.println(e.getMessage());
    }
  }

  public String update(String cbSessionId, UpdateKey updateKey, byte[] content,
		       UpdateQos updateQos)
  {
    System.out.println("\nReceived message: " + updateKey.getOid()
		       + ", state = " + updateQos.getState() + "\n"
		       + "********************************\n");
    String receivedMessage = new String(content);
    System.out.println(receivedMessage);
    try {
      FileWriter fw = new FileWriter("rcv-" + i++ + ".xml");
      fw.write(receivedMessage);
      fw.close();
    }
    catch (IOException e) {
      System.out.println( "Writing to file failed!" );
    }
    return "";
  }

  public static void main(String args[]) {
    String topic = args[args.length-1];
    String arguments[] = new String[args.length-1];
    for (int i = 0; i < args.length-1; i++)
      arguments[i] = args[i];
    Global glob = new Global(); 
    if (glob.init(arguments) != 0) {
      System.err.println("Example: ./startReceiver.sh hostname topic\n");      
      System.exit(1);
    }
    new MessageReceiver(glob, topic);
  }
}
