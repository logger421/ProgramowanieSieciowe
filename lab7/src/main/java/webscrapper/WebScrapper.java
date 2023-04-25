package webscrapper;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;

import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

public class WebScrapper {
    // We will poke to -> "http://th.if.uj.edu.pl/"
    private static URL url;
    private static URLConnection conn;
    private static InputStream inputStream;

    public WebScrapper() {
    }

    public static void main(String[] args) {
        try {
            Document soup = Jsoup.connect("http://th.if.uj.edu.pl/").get();
            url = new URL("http://th.if.uj.edu.pl/");
            conn = url.openConnection();
            inputStream = conn.getInputStream();
            int b;
            while ((b = inputStream.read()) != -1) {
                System.out.print((char) b);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}