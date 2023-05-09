package webscrapper;

import org.jsoup.Connection;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class WebScrapper {
    // We will poke to -> "http://th.if.uj.edu.pl/"
    private static boolean validateHeader(int status, String header) {
        return status == 200 && header.equals("text/html");
    }

    private static StringBuffer readContent(BufferedReader br) throws IOException {
        String inputLine;
        StringBuffer content = new StringBuffer();
        while ((inputLine = br.readLine()) != null) {
            content.append(inputLine);
        }
        return content;
    }

    public static void main(String[] args) {
        HttpURLConnection conn = null;
        BufferedReader br = null;

        try {
            Connection.Response response = Jsoup.connect("http://th.if.uj.edu.pl/").followRedirects(false).execute();
            int statusCode = response.statusCode();
            String statusMessage = response.statusMessage();
            System.out.println(statusCode + " " + statusMessage);
            if(statusCode == 200 && statusMessage.equals("text/html")) {
                Document soup = response.parse();
                String headTag = soup.title();
                System.out.println(soup.title());
            }

            final URL url = new URL("http://th.if.uj.edu.pl/");
            conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");
            conn.setRequestProperty("Content-Type", "text/html");
            String contentType = conn.getHeaderField("Content-Type");
            int status = conn.getResponseCode();
            br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
            if (validateHeader(status, contentType)) {
                StringBuffer content = readContent(br);
                if (content.toString().contains("Institute of Theoretical Physics")){
                    System.out.println("OK");
                    System.exit(0);
                }
                else {
                    System.out.println("FAILURE");
                    System.exit(1);
                }
            }
            else {
                System.out.println("Error return status code: " + status);
                System.exit(1);
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            System.out.println("Clearing");
            if (conn != null) conn.disconnect();
        }
    }
}