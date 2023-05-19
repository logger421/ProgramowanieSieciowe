package org.logger;

import org.jsoup.nodes.Document;
import org.junit.jupiter.api.Assertions;
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
}