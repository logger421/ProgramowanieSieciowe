package org.logger421.discogsrest;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.*;

public class Caller {
    private static final String baseURL = "https://api.discogs.com";
    private static String bandName;

    public static void main(String[] args) {
        if (args.length != 1) System.exit(-1);

        try {
            Integer bandId = Integer.parseInt(args[0]);
            Map<String, Integer> bandMembers = getBandMembers(bandId);
            Map<String, List<String>> allBandsForMembers = getAllBands(bandMembers);
            Map<String, List<String>> sharedMembers = checkSharedMembers(allBandsForMembers);
            printResult(sharedMembers);
        } catch (NumberFormatException ex) {
            System.out.println("Given arg is not valid integer value!");
            System.exit(-1);
        } catch (URISyntaxException | IOException | InterruptedException e) {
            e.printStackTrace();
        }
        System.exit(0);
    }

    private static Map<String, Integer> getBandMembers(Integer bandId) throws IOException, InterruptedException, URISyntaxException {
        String uri = baseURL + "/artists/" + bandId;
        HttpRequest getRequest = HttpRequest
                .newBuilder()
                .uri(new URI(uri))
                .header("user-agent", "Logger421/app/1.0")
                .GET()
                .build();
        HttpClient httpClient = HttpClient.newHttpClient();
        HttpResponse<String> response = httpClient.send(getRequest, HttpResponse.BodyHandlers.ofString());

        if (response.statusCode() != 200) System.exit(-1);

        JSONObject jsonRes = new JSONObject(response.body());
        bandName = jsonRes.getString("name");
        JSONArray members = jsonRes.getJSONArray("members");

        Map<String, Integer> toReturn = new HashMap<>();

        for (int i = 0; i < members.length(); i++) {
            JSONObject member = (JSONObject) members.get(i);
            toReturn.put(member.getString("name"), member.getInt("id"));
        }

        return toReturn;
    }

    private static Map<String, List<String>> getAllBands(Map<String, Integer> bandMembers) {
        Map<String, List<String>> allBands = new TreeMap<>();
        for (var member : bandMembers.entrySet()) {
            allBands.put(member.getKey(), getBandsForMusician(member.getValue()));
        }
        return allBands;
    }

    private static List<String> getBandsForMusician(Integer id) {
        List<String> bands = new ArrayList<>();
        try {
            String uri = baseURL + "/artists/" + id;
            HttpRequest getRequest = HttpRequest
                    .newBuilder()
                    .uri(new URI(uri))
                    .header("user-agent", "Logger421/app/1.0")
                    .GET()
                    .build();
            HttpClient httpClient = HttpClient.newHttpClient();
            HttpResponse<String> response = httpClient.send(getRequest, HttpResponse.BodyHandlers.ofString());
            JSONObject res = new JSONObject(response.body());
            JSONArray jsonBands = res.getJSONArray("groups");
            for (int i = 0; i < jsonBands.length(); i++) {
                JSONObject band = (JSONObject) jsonBands.get(i);
                bands.add(band.getString("name"));
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        return bands;
    }

    private static Map<String, List<String>> checkSharedMembers(Map<String, List<String>> bandMembers) {
        Map<String, List<String>> commonBands = new TreeMap<>();
        for (var entry : bandMembers.entrySet()) {
            String memberName = entry.getKey();
            List<String> bands = entry.getValue();
            for (String band : bands) {
                if(band.equals(bandName))
                    continue;
                List<String> commonMembers = commonBands.getOrDefault(band, new ArrayList<>());
                commonMembers.add(memberName);
                commonBands.put(band, commonMembers);
            }
        }
        // trim result
        commonBands.entrySet().removeIf(entry -> entry.getValue().size() < 2);

        return commonBands;
    }

    private static void printResult(Map<String, List<String>> sharedMembers) {
        if (sharedMembers == null || sharedMembers.isEmpty()) {
            System.out.println("Artists of given band don't share other bands!");
        } else {
            for (var entry : sharedMembers.entrySet()) {
                String toPrint = String.format("%s: %s",
                        entry.getKey(),
                        String.join(",", entry.getValue()));
                System.out.println(toPrint);
            }
        }
    }

}
