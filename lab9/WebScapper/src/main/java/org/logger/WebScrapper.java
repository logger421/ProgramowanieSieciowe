package org.logger;

import org.jsoup.Connection;
import org.jsoup.HttpStatusException;
import org.jsoup.helper.HttpConnection;
import org.jsoup.nodes.Document;
import org.jsoup.select.Elements;

import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class WebScrapper {
    static private String url = "https://www.accuweather.com/en/pl/krakow/274455/weather-forecast/274455";
    static private String windGustsRegex = ".*\\s+(?<target>\\d+\\skm/h)";
    static private String airQualityRegex = "Air\\s+Quality\\s+(?<target>\\w*\\s*)";

    private WebScrapper() {
        // we don't want to instantiate this class
    }

    public static Document getData() {
        try {
            return HttpConnection.connect(url).method(Connection.Method.GET).get();
        } catch (IOException e) {
            if (e instanceof HttpStatusException) {
                System.out.println("Status code != 200");
            } else if (e instanceof java.net.MalformedURLException) {
                System.out.println("Request URL is not a http or https URL");
            } else {
                e.printStackTrace();
            }
            System.exit(-1);
        }
        return null;
    }

    public static String parseDocument(Document doc) throws NullPointerException {
        String formattedOutput = "";
        if (doc == null) {
            throw new NullPointerException("Passed doc argument is null");
        }
        Elements currentWeatherPanel = doc.getElementsByClass("cur-con-weather-card");
        String contentHref = currentWeatherPanel.attr("href");
        if (!contentHref.equals("/en/pl/krakow/274455/current-weather/274455")) {
            formattedOutput = "Parsed content is invalid!";
        }

        String title = currentWeatherPanel.select("h2.cur-con-weather-card__title").text();
        String time = currentWeatherPanel.select("p.cur-con-weather-card__subtitle").text();
        String temp = currentWeatherPanel.select("div.temp").text();
        String windGusts = findMatch(windGustsRegex,
                currentWeatherPanel
                .select("div.details-container div:nth-child(3)")
                .text());
        String airQuality = findMatch(airQualityRegex,
                currentWeatherPanel
                .select("div.details-container div:nth-child(1)")
                .text());

        formattedOutput = String.format("%s,Temperature: %s,Wind gusts: %s,Air quality: %s", time, temp, windGusts,
                airQuality);

        return formattedOutput.isEmpty() ? "ERROR" : formattedOutput;
    }

    private static String findMatch(String regex, String s) {
        Pattern pattern = Pattern.compile(regex);
        Matcher mat = pattern.matcher(s);
        String res = "";
        if (mat.matches()) {
            res = mat.group("target");
        }
        return res == null ? "" : res;
    }

    public static void main(String[] args){
        System.out.println("Weather shortcut for city: Cracow");
        Document doc = getData();
        try {
            String result = parseDocument(doc);
            System.out.println(result);
        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(-1);
        }
        System.exit(0);
    }
}
