package org.logger;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import javax.management.BadAttributeValueExpException;
import javax.swing.text.html.parser.Parser;
import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.charset.CharsetDecoder;
import java.nio.file.Path;
import java.nio.file.Paths;


class WebScrapperTest {

    @Test
    void shouldGetDocument() {
        Document doc = WebScrapper.getData();
        Assertions.assertNotNull(doc);
    }

    @Test
    void shouldThrowExceptionWhenDocIsNull(){
        try {
            WebScrapper.parseDocument(null);
            Assertions.fail("Exception wasn't thrown!");
        } catch (NullPointerException e) {
            Assertions.assertEquals("Passed doc argument is null", e.getMessage());
        }
    }

    @Test
    void shouldReturnCorrectResult(){
        try {
            URL resource = WebScrapperTest.class.getResource("/web_snapshot.html");
            Document doc = Jsoup.parse(Paths.get(resource.toURI()).toFile());
            System.out.println(doc.title());
            Assertions.assertEquals("11:06 PM,Temperature: 13°C,Wind gusts: 13 km/h,Air quality: Fair",
                    WebScrapper.parseDocument(doc));
        } catch (NullPointerException e) {
            e.printStackTrace();
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (URISyntaxException e) {
            throw new RuntimeException(e);
        }
    }
}