# Order Book Matching Engine

C++ implementation of a order book, similar to those used in stock exchanges and crypto trading platforms.  
**limit orders, market orders, order modification, cancellation, and matching** under price–time priority.

## Features

- **Limit Orders**: Place buy/sell orders at a specific price.
- **Market Orders**: Consume available liquidity immediately at the best price.
- **Order Matching**: Matches opposing orders when bids cross asks, using price–time priority.
- **Order Modification**: Update the quantity of an existing order.
- **Order Cancellation**: Remove an order by ID, cleaning up empty price levels.
- **Price Levels & Depth Tracking**: Maintains total liquidity at each price level.
- **Trade Logging**: Outputs executed trades (`quantity @ price`) to console.

  
