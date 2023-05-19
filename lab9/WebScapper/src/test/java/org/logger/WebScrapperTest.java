package org.logger;

import org.jsoup.nodes.Document;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import javax.management.BadAttributeValueExpException;


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
        } catch (BadAttributeValueExpException e) {
            Assertions.assertEquals("BadAttributeValueException: Passed doc argument is null", e.toString());
        }
    }

    @Test
    @Disabled
    void shouldReturnCorrectResult(){
        try {
            // Use mock or stub to handle external source of data. Time and Temperature will differ between tests, but test should check data parsing correctness.
            Assertions.assertEquals("11:27 AM: Temperature: 11°C,Humidity: ,Wind gusts: 13 km/h,Air quality:", WebScrapper.parseDocument(WebScrapper.getData()));
        } catch (BadAttributeValueExpException e) {
            throw new RuntimeException(e);
        }
    }
}