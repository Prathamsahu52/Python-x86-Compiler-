class Customer:
    def __init__(self, name, age):
        self.name = name
        self.age = age
        self.orders = []

    def add_order(self, order):
        self.orders.append(order)

    def get_name(self):
        return self.name

    def get_age(self):
        return self.age

    def get_orders(self):
        return self.orders

class Order:
    def __init__(self, order_id, products):
        self.order_id = order_id
        self.products = products

    def get_order_id(self):
        return self.order_id

    def get_products(self):
        return self.products

class Inventory:
    def __init__(self):
        self.items = []
        self.quantities = []

    def add_item(self, item, quantity):
        if item in self.items:
            index = self.items.index(item)
            self.quantities[index] += quantity
        else:
            self.items.append(item)
            self.quantities.append(quantity)

    def remove_item(self, item, quantity):
        if item in self.items:
            index = self.items.index(item)
            if self.quantities[index] >= quantity:
                self.quantities[index] -= quantity
                return True
            else:
                print(f"Not enough {item} in stock.")
                return False
        else:
            print(f"{item} not found in stock.")
            return False

    def check_stock(self, item):
        if item in self.items:
            index = self.items.index(item)
            return self.quantities[index]
        else:
            return 0

class Store:
    def __init__(self):
        self.customers = []
        self.inventory = Inventory()

    def add_customer(self, customer):
        self.customers.append(customer)

    def process_order(self, customer_name, order_id):
        for customer in self.customers:
            if customer.get_name() == customer_name:
                for order in customer.get_orders():
                    if order.get_order_id() == order_id:
                        total_price = 0
                        for product in order.get_products():
                            if self.inventory.check_stock(product) > 0:
                                total_price += self.get_product_price(product)
                            else:
                                print(f"{product} is out of stock.")
                                return False
                        if self.check_customer_balance(customer) >= total_price:
                            for product in order.get_products():
                                self.inventory.remove_item(product, 1)
                            self.update_customer_balance(customer, total_price)
                            return True
                        else:
                            print("Insufficient balance.")
                            return False
        print("Customer or order not found.")
        return False

    def get_product_price(self, product):
        if "Apple" in product:
            return 2
        elif "Laptop" in product or "Phone" in product:
            return 1000
        else:
            return 10

    def check_customer_balance(self, customer):
        # Dummy function to check customer balance
        return 1000

    def update_customer_balance(self, customer, amount):
        # Dummy function to update customer balance
        print("Dedicated", amount, "from", customer.get_name(), "s account.")

def main():
    store = Store()

    customer1 = Customer("Alice", 30)
    customer2 = Customer("Bob", 25)

    order1 = Order(101, ["Apple", "Banana", "Orange"])
    order2 = Order(102, ["Laptop", "Phone"])
    order3 = Order(103, ["Book", "Pen"])

    customer1.add_order(order1)
    customer1.add_order(order2)
    customer2.add_order(order3)

    store.add_customer(customer1)
    store.add_customer(customer2)

    store.inventory.add_item("Apple", 100)
    store.inventory.add_item("Laptop", 50)
    store.inventory.add_item("Pen", 200)

    # Process orders
    store.process_order("Alice", 101)
    store.process_order("Bob", 103)

if __name__ == "__main__":
    main()
