package org.logger;

import org.jsoup.Connection;
import org.jsoup.HttpStatusException;
import org.jsoup.helper.HttpConnection;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import javax.management.BadAttributeValueExpException;
import java.io.IOException;
import java.net.URL;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class WebScrapper {
    static private String url = "https://www.accuweather.com/en/pl/krakow/274455/weather-forecast/274455";


    private WebScrapper() {
        // we don't want to instantiate this class
    }

    public static Document getData() {
        try {
            return HttpConnection.connect(url).method(Connection.Method.GET).get();
        } catch (IOException e) {
            if (e instanceof HttpStatusException) {
                System.out.println("Status code != 200");
            }
            if (e instanceof java.net.MalformedURLException) {
                System.out.println("Request URL is not a http or https URL");
            } else {
                e.printStackTrace();
            }
            System.exit(-1);
        }
        return null;
    }

    public static String parseDocument(Document doc) throws BadAttributeValueExpException {
        String formattedOutput = "";
        if (doc == null) {
            throw new BadAttributeValueExpException("Passed doc argument is null");
        }
        Elements currentWeatherPanel = doc.getElementsByClass("cur-con-weather-card");
        String contentHref = currentWeatherPanel.attr("href");
        if (contentHref.equals("/en/pl/krakow/274455/current-weather/274455")) {
            String title = currentWeatherPanel.select("h2.cur-con-weather-card__title").text();
            String time = currentWeatherPanel.select("p.cur-con-weather-card__subtitle").text();
            String temp = currentWeatherPanel.select("div.temp").text();
            String windGusts = matchWindGusts(currentWeatherPanel
                    .select("div.details-container div:nth-child(3)")
                    .text());
//                String humidity = currentWeatherPanel.select().text();
            System.out.format("%s; %s: temperature: %s; humidity: ;wind gusts: %s;air quality: \n",
                    time, title, temp, windGusts);
        }
        return formattedOutput.equals("") ? "ERROR" : formattedOutput;
    }

    private static String matchWindGusts(String s) {
        String regex = ".*\\s+(?<speed>\\d+\\skm/h)";
        Pattern pattern = Pattern.compile(regex);
        Matcher mat = pattern.matcher(s);
        if (mat.matches()) {
            return mat.group("speed");
        }
        return "";
    }

    public static void main(String[] args){
        Document doc = getData();
        try {
            parseDocument(doc);
        } catch (Exception ex) {
            System.exit(-1);
        }
        System.exit(0);
    }
}