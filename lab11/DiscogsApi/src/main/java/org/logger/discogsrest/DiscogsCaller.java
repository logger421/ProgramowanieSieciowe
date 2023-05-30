package org.logger.discogsrest;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;

public class DiscogsCaller {
    public static void main(String[] args) {
        try {
            HttpRequest getRequest =
                    HttpRequest
                    .newBuilder()
                    .uri(new URI("https://api.discogs.com/artists/359282/releases"))
                    .header("user-agent", "Logger421/app/1.0").GET().build();
            HttpClient httpClient = HttpClient.newHttpClient();
            HttpResponse<String> response = httpClient.send(getRequest, HttpResponse.BodyHandlers.ofString());
            System.out.println(response.statusCode());
            JSONObject object = new JSONObject(response.body());
            JSONArray albums = object.getJSONArray("releases");
            System.out.println(albums.length());
            for(int i = 0; i < albums.length(); i++) {
                JSONObject album = albums.getJSONObject(i);
                String printAlbumInfo = String.format("%s (%s): %s",
                        album.getString("artist"),
                        album.getInt("year"),
                        album.getString("title"));
                System.out.println(printAlbumInfo);
            }

        } catch (URISyntaxException | IOException | InterruptedException e) {
            throw new RuntimeException(e);
        }
    }
}
